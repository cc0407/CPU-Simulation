#ifndef SIMCPU
#define SIMCPU

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum STATE {
    NEW, READY, RUNNING, BLOCKED, TERMINATED
} state;

typedef struct {
    int burstNo;
    int cpuTime;
    int ioTime;
} cpuBurst;

typedef struct {
    int arrTime; // Arrival Time
    int finTime; // Finish Time
    int turnTime; // Turnaround Time
    int serTime; // Service Time
    int PNo; // Process Number
    int TNo; // Thread Number
    int burstNo; // Number of bursts
    cpuBurst** bursts;
    state s;
} thread;

typedef struct {
    thread** threads;
    int threadAmt; // # of threads in the threads array
} process;

typedef struct {
    int key;
    void* data;
} node;

typedef struct {
    node* harr;
    int curr_size;
} heap;


heap* initializePriorityQueue(process*** p, int* processAmt, int* threadSwitch, int* processSwitch) ;
thread** createThreadList( int pNum, int tAmt );
cpuBurst** createBurstList(int burstAmt, int tNum);

/* Process/Thread Helper Functions */
int getTotalIOTime(thread* t);
float getAverageTurnaroundTime(process** processes, int processAmt);
thread* createEmptyThread();
void printProcesses(process** processes, int processAmt);
void printThreads(thread** threads, int threadAmt);
void freeProcesses(process*** processes, int processAmt); // Frees a list of processes
void freeThreads( thread** threads, int threadAmt);
void freeBursts(cpuBurst** bursts, int burstAmt);

/* Heap Functions */
heap* initializeHeap();
void insertItem(heap* h, int key, void* data);
void* removeMin(heap* h); // Removes and returns top node

int minKey(heap* h); // Gets the arrival time of the top node
node* minElement(heap* h); // Gets a whole thread from the top node
int heapSize(heap* h); // Returns the total size of a given heap
bool isEmpty(heap* h);

void upheap(heap* h); // Restores heap-order after insertion MUST BE CALLED IN INSERT
void downheap(heap* h); // Restores heap-order after removal MUST BE CALLED IN REMOVE

#endif /* SIMCPU */