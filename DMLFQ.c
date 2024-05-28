#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// Define task structure
struct Task {
    int id;
    int arrival_time;
    int execution_time;
    int deadline;
    int priority;
    int start_time;
    int end_time;
    int wait_time;
    int turnaround_time;
    int remaining_time;
};

// Define queues for each level
struct Task real_time_queue[100];
int real_time_front = 0, real_time_rear = -1;

struct Task system_process_queue[100];
int system_process_front = 0, system_process_rear = -1;

struct Task interactive_process_queue[100];
int interactive_process_front = 0, interactive_process_rear = -1;

struct Task batch_process_queue[100];
int batch_process_front = 0, batch_process_rear = -1;

// Time quantums for each level
const int time_quantum[] = {16, 8, 4, 2};  // Batch, Interactive, System, Real-Time

// Function prototypes
void add_task(struct Task new_task);
void enqueue(struct Task queue[], int *rear, struct Task task);
struct Task dequeue(struct Task queue[], int *front, int *rear);
bool is_empty(int front, int rear);
void *process_real_time_queue(void *arg);
void *process_system_queue(void *arg);
void *process_interactive_queue(void *arg);
void *process_batch_queue(void *arg);

// Global variables
int current_time = 0;
int total_tasks = 0;
int completed_tasks = 0;
int total_wait_time = 0;
int total_turnaround_time = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to add a new task to the appropriate queue
void add_task(struct Task new_task) {
    if (new_task.deadline < new_task.arrival_time + new_task.execution_time) {
        printf("Error: Task deadline is not feasible!\n");
        return;
    }

    new_task.remaining_time = new_task.execution_time;
    pthread_mutex_lock(&mutex);
    total_tasks++;
    pthread_mutex_unlock(&mutex);

    switch (new_task.priority) {
        case 3:
            enqueue(real_time_queue, &real_time_rear, new_task);
            break;
        case 2:
            enqueue(system_process_queue, &system_process_rear, new_task);
            break;
        case 1:
            enqueue(interactive_process_queue, &interactive_process_rear, new_task);
            break;
        case 0:
            enqueue(batch_process_queue, &batch_process_rear, new_task);
            break;
        default:
            printf("Error: Invalid priority!\n");
            break;
    }
}

// Function to enqueue a task into the appropriate queue
void enqueue(struct Task queue[], int *rear, struct Task task) {
    (*rear)++;
    queue[*rear] = task;
}

// Function to dequeue a task from the appropriate queue
struct Task dequeue(struct Task queue[], int *front, int *rear) {
    struct Task task = queue[*front];
    (*front)++;
    return task;
}

// Function to check if a queue is empty
bool is_empty(int front, int rear) {
    return front > rear;
}

// Function to process tasks in the real-time queue using EDF scheduling
void *process_real_time_queue(void *arg) {
    while (completed_tasks < total_tasks) {
        pthread_mutex_lock(&mutex);
        if (!is_empty(real_time_front, real_time_rear)) {
            struct Task next_task = dequeue(real_time_queue, &real_time_front, &real_time_rear);
            if (next_task.arrival_time <= current_time) {
                int execution_time = next_task.remaining_time < time_quantum[3] ? next_task.remaining_time : time_quantum[3];
                next_task.start_time = current_time;
                next_task.end_time = current_time + execution_time;
                next_task.remaining_time -= execution_time;

                if (next_task.remaining_time > 0) {
                    enqueue(real_time_queue, &real_time_rear, next_task);
                } else {
                    next_task.wait_time = next_task.start_time - next_task.arrival_time;
                    next_task.turnaround_time = next_task.end_time - next_task.arrival_time;
                    total_wait_time += next_task.wait_time;
                    total_turnaround_time += next_task.turnaround_time;
                    completed_tasks++;
                }

                printf("[Time %d] Task %d (Priority %d) starts execution\n", current_time, next_task.id, next_task.priority);
                current_time = next_task.end_time;
                printf("[Time %d] Task %d (Priority %d) ends execution\n", current_time, next_task.id, next_task.priority);
                printf("Task %d (Priority %d) took %d time units\n", next_task.id, next_task.priority, execution_time);
            } else {
                enqueue(real_time_queue, &real_time_rear, next_task);
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000);  // Sleep for a short while to simulate time passing
    }
    return NULL;
}

// Function to process tasks in the system queue using Round Robin scheduling
void *process_system_queue(void *arg) {
    while (completed_tasks < total_tasks) {
        pthread_mutex_lock(&mutex);
        if (!is_empty(system_process_front, system_process_rear)) {
            struct Task next_task = dequeue(system_process_queue, &system_process_front, &system_process_rear);
            if (next_task.arrival_time <= current_time) {
                int execution_time = next_task.remaining_time < time_quantum[2] ? next_task.remaining_time : time_quantum[2];
                next_task.start_time = current_time;
                next_task.end_time = current_time + execution_time;
                next_task.remaining_time -= execution_time;

                if (next_task.remaining_time > 0) {
                    enqueue(system_process_queue, &system_process_rear, next_task);
                } else {
                    next_task.wait_time = next_task.start_time - next_task.arrival_time;
                    next_task.turnaround_time = next_task.end_time - next_task.arrival_time;
                    total_wait_time += next_task.wait_time;
                    total_turnaround_time += next_task.turnaround_time;
                    completed_tasks++;
                }

                printf("[Time %d] Task %d (Priority %d) starts execution\n", current_time, next_task.id, next_task.priority);
                current_time = next_task.end_time;
                printf("[Time %d] Task %d (Priority %d) ends execution\n", current_time, next_task.id, next_task.priority);
                printf("Task %d (Priority %d) took %d time units\n", next_task.id, next_task.priority, execution_time);
            } else {
                enqueue(system_process_queue, &system_process_rear, next_task);
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000);  // Sleep for a short while to simulate time passing
    }
    return NULL;
}

// Function to process tasks in the interactive queue using LST scheduling
void *process_interactive_queue(void *arg) {
    while (completed_tasks < total_tasks) {
        pthread_mutex_lock(&mutex);
        if (!is_empty(interactive_process_front, interactive_process_rear)) {
            struct Task next_task = dequeue(interactive_process_queue, &interactive_process_front, &interactive_process_rear);
            if (next_task.arrival_time <= current_time) {
                int execution_time = next_task.remaining_time < time_quantum[1] ? next_task.remaining_time : time_quantum[1];
                next_task.start_time = current_time;
                next_task.end_time = current_time + execution_time;
                next_task.remaining_time -= execution_time;

                if (next_task.remaining_time > 0) {
                    enqueue(interactive_process_queue, &interactive_process_rear, next_task);
                } else {
                    next_task.wait_time = next_task.start_time - next_task.arrival_time;
                    next_task.turnaround_time = next_task.end_time - next_task.arrival_time;
                    total_wait_time += next_task.wait_time;
                    total_turnaround_time += next_task.turnaround_time;
                    completed_tasks++;
                }

                printf("[Time %d] Task %d (Priority %d) starts execution\n", current_time, next_task.id, next_task.priority);
                current_time = next_task.end_time;
                printf("[Time %d] Task %d (Priority %d) ends execution\n", current_time, next_task.id, next_task.priority);
                printf("Task %d (Priority %d) took %d time units\n", next_task.id, next_task.priority, execution_time);
            } else {
                enqueue(interactive_process_queue, &interactive_process_rear, next_task);
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000);  // Sleep for a short while to simulate time passing
    }
    return NULL;
}

// Function to process tasks in the batch queue using FCFS scheduling
void *process_batch_queue(void *arg) {
    while (completed_tasks < total_tasks) {
        pthread_mutex_lock(&mutex);
        if (!is_empty(batch_process_front, batch_process_rear)) {
            struct Task next_task = dequeue(batch_process_queue, &batch_process_front, &batch_process_rear);
            if (next_task.arrival_time <= current_time) {
                int execution_time = next_task.remaining_time < time_quantum[0] ? next_task.remaining_time : time_quantum[0];
                next_task.start_time = current_time;
                next_task.end_time = current_time + execution_time;
                next_task.remaining_time -= execution_time;

                if (next_task.remaining_time > 0) {
                    enqueue(batch_process_queue, &batch_process_rear, next_task);
                } else {
                    next_task.wait_time = next_task.start_time - next_task.arrival_time;
                    next_task.turnaround_time = next_task.end_time - next_task.arrival_time;
                    total_wait_time += next_task.wait_time;
                    total_turnaround_time += next_task.turnaround_time;
                    completed_tasks++;
                }

                printf("[Time %d] Task %d (Priority %d) starts execution\n", current_time, next_task.id, next_task.priority);
                current_time = next_task.end_time;
                printf("[Time %d] Task %d (Priority %d) ends execution\n", current_time, next_task.id, next_task.priority);
                printf("Task %d (Priority %d) took %d time units\n", next_task.id, next_task.priority, execution_time);
            } else {
                enqueue(batch_process_queue, &batch_process_rear, next_task);
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000);  // Sleep for a short while to simulate time passing
    }
    return NULL;
}

// Main function
int main() {
    int num_tasks;
    printf("Enter the number of tasks: ");
    scanf("%d", &num_tasks);

    for (int i = 0; i < num_tasks; i++) {
        struct Task new_task;
        printf("Enter details for Task %d:\n", i + 1);
        printf("ID: ");
        scanf("%d", &new_task.id);
        printf("Arrival Time: ");
        scanf("%d", &new_task.arrival_time);
        printf("Execution Time: ");
        scanf("%d", &new_task.execution_time);
        printf("Deadline: ");
        scanf("%d", &new_task.deadline);
        printf("Priority (0 for batch, 1 for interactive process, 2 for system process, 3 for real-time): ");
        scanf("%d", &new_task.priority);

        add_task(new_task);
    }

    pthread_t real_time_thread, system_thread, interactive_thread, batch_thread;
    pthread_create(&real_time_thread, NULL, process_real_time_queue, NULL);
    pthread_create(&system_thread, NULL, process_system_queue, NULL);
    pthread_create(&interactive_thread, NULL, process_interactive_queue, NULL);
    pthread_create(&batch_thread, NULL, process_batch_queue, NULL);

    pthread_join(real_time_thread, NULL);
    pthread_join(system_thread, NULL);
    pthread_join(interactive_thread, NULL);
    pthread_join(batch_thread, NULL);

    printf("\nAverage Wait Time: %.2f\n", (float)total_wait_time / total_tasks);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / total_tasks);

    return 0;
}