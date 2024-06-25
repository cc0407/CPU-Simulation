# CPU-Simulation
A simulation for a CPU scheduling algorithm that will complete the execution of a group of multi-threaded processes in an OS that understands threads (kernel threads).

Each process has 1-50 threads; each of the threads has its own CPU and I/O requirements.  
The simulation scheduling policy is on the **thread** level. The CPU scheduling algorithm doesn't use future knowledge in the scheduling algorithm

## Specifications
Given a set of processes to execute with CPU and I/O requirements, the program will simulate the execustion of the threads based on the following scheduling policies:  
- FCFS (First Come First Serve)
- RR (Round Robin)

The simulation will also collect the following statistics:
- The total time required to execute all the threads in all the processes
- The CPU utilization
- The average turn-around time for all processes
- The service time (or CPU time), I/O time, and turnaround time for each individual thread

## Simulation Structure
The program is built using next even simulation. At any given time, the simulation is in a single state. The simulation state can only change at event times, where an event is defined as an occurence that may change the state of the system.  
Events are scheduled via an event queue - a sorted queue which contains future events. The queue is sorted by the time of these future events.

### Examples of events
- Thread arrival
- Transition of thread state

## Input file format
The simulation input includes:
- The number of process that require execution
- The thread switch overhead time (the number of time units required to switch to a new thread in the SAME process)
- The process switch overhead time (the number of time units required to switch to a new thread in a DIFFERENT process)
- For each process
  - The number of threads
  - The arrival time for each thread
  - The number of CPU execution bursts each thread requires (the CPU execution bursts are separated by the time it takes for the thread to do I/O)
  - The cpu time and the I/O time
 
### Example Input File Structure
```
number_of_processes thread_switch process_switch  
process_number(1) number_of_threads(1)  
    thread_number(1) arrival_time(1) number_of_CPU(1)  
        1 cpu_time io_time  
        2 cpu_time io_time  
        ...  
        number_of_CPU(1) cpu_time  
    thread_number(2) arrival_time(2) number_of_CPU(2)  
        1 cpu_time io_time  
        2 cpu_time io_time  
        ...  
        number_of_CPU(2) cpu_time ...  
    ...  
    thread_number(n) arrival_time(n) number_of_CPU(n)  
        1 cpu_time io_time  
        2 cpu_time io_time  
        ...  
        number_of_CPU(n) cpu_time  
process_number(2) number_of_threads(2)  
    thread_number(1) arrival_time(1) number_of_CPU(1)  
        1 cpu_time io_time  
        2 cpu_time io_time  
        ...  
        number_of_CPU(1) cpu_time  
```
### Example Input File
An input file `testfile` is included in the repo:
```
2 3 7  
1 2  
1 0 3  
1 10 20  
2 10 20  
3 10  
2 5 2  
1 50 10  
2 50  
2 2  
1 0 1  
1 100  
2 50 2  
1 100 20  
2 100
```

## Compilation Instructions
- Navigate to the root directory  
- Run `make`

## Running the simulation
- Navigate to the root directory  
- Create an input file that matches the format provided in the assignment PDF  
    - Input file is validated on program launch
    - A test file `testfile.txt` is provided in the root directory
- run `./simcpu [-d] [-v] [-r] [quantum] < [inputfile]`, where
    - `[-d]` toggles detailed mode
    - `[-v]` toggles verbose mode
    - `[-r] [quantum]` toggles round robin mode with a time quantum of `[quantum]` time units
    - `[inputfile]` is the input file created in the previous step
    - input parameters are validated on program launch
