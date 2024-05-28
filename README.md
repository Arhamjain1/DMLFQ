# DMLFQ

### DMLFQ Scheduler

This program implements a Multi-Level Feedback Queue (MLFQ) scheduler in C using pthreads for concurrent task processing. Tasks are categorized into four priority levels: real-time, system process, interactive process, and batch process. Each priority level is associated with a queue, and tasks are executed based on their priority using different scheduling algorithms.

#### Features:
- **Task Prioritization**: Tasks are classified into different priority levels based on their characteristics, ensuring efficient resource allocation.
- **Concurrent Processing**: Multi-threading is employed to process tasks from different priority levels simultaneously, maximizing system throughput.
- **Scheduling Algorithms**:
  - **Real-Time Queue**: Earliest Deadline First (EDF) scheduling to prioritize tasks with the nearest deadlines.
  - **System Process Queue**: Round Robin (RR) scheduling for fair execution of system-related tasks.
  - **Interactive Process Queue**: Shortest Time-to-Completion (LST) scheduling to ensure responsive execution of interactive tasks.
  - **Batch Process Queue**: First-Come, First-Served (FCFS) scheduling for sequential execution of batch tasks.

#### Usage:
1. Compile the program using any C compiler.
2. Run the executable and follow the prompts to input task details, including arrival time, execution time, deadline, and priority.
3. The program will simulate task execution, displaying the start and end times for each task and calculating average wait time and turnaround time.

#### Requirements:
- C compiler (e.g., GCC)
- POSIX Threads (pthreads) library
