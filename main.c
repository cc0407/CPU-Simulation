#include "main.h"
int detailed = false;
int verbose = 0;
int RRTime = 0;

int main(int argc, char* argv[]) {
    process*** processes;
    int processAmt;
    int threadSwitch;
    int processSwitch;
    float timeElapsed = 0;
    float CPUUtilizationTime = 0;
    heap* h;

    processes = (process***)malloc(1 * sizeof(process**));
    *processes = NULL;
    h = initializePriorityQueue(processes, &processAmt, &threadSwitch, &processSwitch);

    // Input flag parsing
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-d") == 0) { // -d flag was presented
            detailed = true;
        }
        if(strcmp(argv[i], "-v") == 0) { // -d flag was presented
            verbose = true;
        }
        if(strcmp(argv[i], "-r") == 0) { // -r flag was presented
            i++;
            if(i >= argc) {
                fprintf(stderr, "A time quantum is required with -r flag.\nUsage: simcpu [-d] [-r] [quantum] < input_file\n");
                freeProcesses(processes, processAmt);
                return(1);
            }

            RRTime = atoi(argv[i]);
            if(RRTime == 0) {
                fprintf(stderr, "please indicate a numeric, positive time quantum.\nUsage: simcpu [-d] [-r] [quantum] < input_file\n");
                freeProcesses(processes, processAmt);
                return(1);
            }
        }
    }

    //TODO DEBUGGING
    //printf("Detailed: %d\nVerbose: %d\nRound Robin: %d\n", detailed, verbose, RRTime);

    while( !isEmpty(h) ) {
        //TODO EXTRACT EVENT FROM QUEUE
        //TODO UPDATE TIME TO MATCH EVENT
        //TODO Switch statement for type of event
            //TODO PROCESS EVENT, POSSIBLY ADD MORE EVENTS TO THE QUEUE
            //TODO COLLECT STATS
            
    }

    if(RRTime == 0)
        printf("FCFS Scheduling\n");
    else
        printf("Round Robin Scheduling (quantum = %d time units)\n", RRTime);

    printf("Total Time required is %.0f units\n", timeElapsed);
    printf("Average Turnaround Time is %.1f time units\n", getAverageTurnaroundTime(*processes, processAmt));
    if(timeElapsed != 0)
        printf("CPU Utilization is %d%%\n", CPUUtilizationTime / timeElapsed * 100);
    else
        printf("CPU Utilization is 0%%\n");

    freeProcesses(processes, processAmt);
    return 0;
}

// TODO Read all Processes / Threads / CPU Bursts into their own arrays
// TODO Turn those arrays into a heap measuring arrival time
heap* initializePriorityQueue(process*** p, int* processAmt, int* threadSwitch, int* processSwitch) {
    
    return NULL; //TODO TEMP
}

// Frees a list of processes
void freeProcesses(process*** processes, int processAmt) {
    if(*processes != NULL) {
        for( int i = 0; i < processAmt; i++) {
            freeThreads((*processes)[i]->threads, (*processes)[i]->threadAmt);
            free( (*processes)[i] );
        }
        free(*processes);
    }
    free(processes);
}

// Frees a list of threads
void freeThreads( thread** threads, int threadAmt) {
    if(threads != NULL) {
        for( int i = 0; i < threadAmt; i++) {
            freeBursts(threads[i]->bursts, threads[i]->burstNo);
            free( threads[i]);
        }
        free(threads);
    }
}

// Frees a list of bursts
void freeBursts(cpuBurst** bursts, int burstAmt) {
    if(bursts != NULL) {
        for( int i = 0; i < burstAmt; i++) {
            free( bursts[i]);
        }
        free(bursts);
    }
}

float getAverageTurnaroundTime(process** processes, int processAmt) {
    float avgTime = 0;
    float threadAvgTime;
    if(processes != NULL) {
        for(int i = 0; i < processAmt; i++) { // Iterate through each process
            threadAvgTime = 0;
            for(int j = 0; j < processes[i]->threadAmt; j++) { // Iterate through each thread
                threadAvgTime += processes[i]->threads[i]->turnTime;
            }
            threadAvgTime /= processes[i]->threadAmt;
            avgTime += threadAvgTime;
        }
    }
    return avgTime;
}

bool isEmpty(heap* h) {
    if(h == NULL) {
        return true;
    }
    return h->curr_size == 0;
}