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

    processes = (process***)calloc(1, sizeof(process**));

    h = initializePriorityQueue(processes, &processAmt, &threadSwitch, &processSwitch);
    if(!h) { // heap was not initialized correctly, exiting program
        fprintf(stderr, "Min-heap not initialized correctly. Exiting...\n");
        freeProcesses(processes, processAmt);
        return 1;
    }   

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

    printProcesses(*processes, processAmt);
    freeProcesses(processes, processAmt);
    return 0;
}

// TODO Read all Processes / Threads / CPU Bursts into their own arrays
// TODO Turn those arrays into a heap measuring arrival time
heap* initializePriorityQueue(process*** p, int* processAmt, int* threadSwitch, int* processSwitch) {
    if( scanf(" %d %d %d", processAmt, threadSwitch) != 3 || !validateLineEnding()) {
        fprintf(stderr, "Error ingesting line 0 of input file.\n");
        return NULL;
    }
    //TODO DEBUGGING
    //printf("processAmt: %d\nInternal Time: %d\nExternal Time: %d\n", *processAmt, *threadSwitch, *processSwitch);
    heap* h = initializeHeap();

    *p = (process**)calloc(*processAmt, sizeof(process*)); // Create process array
    int pNum;
    int tAmt;
    for(int i = 0; i < *processAmt; i++) {
        process* newP = (process*)calloc(1, sizeof(process)); // Create local pointer and add it to process list
        (*p)[i] = newP; // Create Process inside of process array
        
        if( scanf(" %d %d", &pNum, &tAmt) != 2 || !validateLineEnding()) {
            fprintf(stderr, "Error ingesting process %d.\n", i+1);
            return NULL;
        }
        newP->threadAmt = tAmt;
        newP->threads = createThreadList( pNum, tAmt);
        if(newP->threads == NULL) { 
            return NULL; 
        }        
    }

    //TODO TEMP
    printProcesses(*p, pNum);
    return h;
}

// Creates a list of threads using data from stdin
thread** createThreadList( int pNum, int tAmt ) {
    thread** tList = (thread**)calloc(tAmt, sizeof(thread*));
    if(!tList) { return NULL; } // NULL Checks for failed malloc

    thread* newThread;
    int tNo;
    int arrTime;
    int bNo;
    for(int i = 0; i < tAmt; i++) {
        newThread = createEmptyThread();  // Create local pointer and add it to thread list
        tList[i] = newThread;
        
        if(!newThread || scanf(" %d %d %d", &tNo, &arrTime, &bNo) != 3  || !validateLineEnding()) { // NULL Checks thread and attempts to ingest more info from stdin
        //if(!newThread || scanf(" %d %d %d ", newThread->TNo, newThread->arrTime, &bNo) != 3 ) {
            fprintf(stderr, "Error ingesting thread %d of process %d.\n", i+1, pNum);
            freeThreads(tList, tAmt);
            return NULL;
        }

        newThread->TNo = tNo;
        newThread->arrTime = arrTime;
        newThread->burstNo = bNo;
        newThread->PNo = pNum;
        newThread->bursts = createBurstList(newThread->burstNo, i+1);
        if(newThread->bursts == NULL) { 
            freeThreads(tList, tAmt);
            return NULL; 
        }
        
    }

    return tList;
}

// Creates a list of bursts using data from stdin
cpuBurst** createBurstList(int bAmt, int tNum) {
    cpuBurst** bList = (cpuBurst**)calloc(bAmt, sizeof(cpuBurst*));
    if(!bList) { return NULL; } // NULL Checks for failed malloc



    cpuBurst* newBurst;
    int burstNo;
    int cpuTime;
    int ioTime;
    for(int i = 0; i < bAmt - 1; i++) {
        newBurst = (cpuBurst*)malloc(1* sizeof(cpuBurst)); // Create local pointer and add it to burst list
        bList[i] = newBurst;

        if(!newBurst || scanf(" %d %d %d", &burstNo, &cpuTime, &ioTime) != 3 || !validateLineEnding()) { // NULL Checks burst and attempts to ingest more info from stdin
            fprintf(stderr, "Error ingesting burst %d of thread %d.\n", i+1, tNum);
            freeBursts(bList, bAmt);
            return NULL;
        }        
        newBurst->burstNo = burstNo;
        newBurst->cpuTime = cpuTime;
        newBurst->ioTime = ioTime;

        
    }

    // Final burst, no IO
    newBurst = (cpuBurst*)malloc(1* sizeof(cpuBurst));
    bList[bAmt - 1] = newBurst;
    if(!newBurst || scanf(" %d %d", &burstNo, &cpuTime) != 2 || !validateLineEnding()) { // NULL Checks burst and attempts to ingest more info from stdin
        fprintf(stderr, "Error ingesting burst %d of thread %d.\n", bAmt, tNum);
        freeBursts(bList, bAmt); // Frees burst list
        return NULL;
    }        
    newBurst->burstNo = burstNo;
    newBurst->cpuTime = cpuTime;
    newBurst->ioTime = 0;

    return bList;
}

// Initializes an empty thread and returns it
thread* createEmptyThread() {
    thread* newThread;
    newThread = (thread*)malloc(sizeof(thread));
    if(!newThread) { return NULL; } // NULL Check for failed malloc
    newThread->PNo = -1;
    newThread->TNo = -1;
    newThread->arrTime = -1; // Arrival Time
    newThread->finTime = -1; // Finish Time
    newThread->turnTime = -1; // Turnaround Time
    newThread->serTime = -1; // Service Time
    newThread->bursts = NULL;
    newThread->s = NEW;

    return newThread;
}

void printProcesses(process** processes, int processAmt) {
    if (processes != NULL) {
        for(int i = 0; i < processAmt; i++) {
            if(processes[i] != NULL)
                printThreads(processes[i]->threads, processes[i]->threadAmt);
        }
    }   
    else {
        printf("No Processes Provided.\n");
    }
}

void printThreads(thread** threads, int threadAmt) {
    thread* currThread;
    if(threads != NULL) {
        for(int i = 0; i < threadAmt; i++) {
            if(threads[i] != NULL) {
                currThread = threads[i];
                printf("Thread %d of Process %d:\n", currThread->TNo, currThread->PNo);
                printf("\tarrival time: %d\n", currThread->arrTime);
                printf("\tservice time: %d units, I/O time: %d units, turnaround time: %d units, finish time: %d units\n", 
                    currThread->serTime, getTotalIOTime(currThread), currThread->turnTime, currThread->finTime);
            }
        }
    }  
    else {
        return;
    }
}

int getTotalIOTime(thread* t) {
    int sum = 0;
    if ( t != NULL ) {
        for(int i = 0; i < t->burstNo; i++) {
            sum+= t->bursts[i]->ioTime;
        }
        return sum;
    }
    else
        return sum;
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

// Frees a list of processes
void freeProcesses(process*** processes, int processAmt) {
    if(*processes != NULL) {
        for( int i = 0; i < processAmt; i++) {
            if((*processes)[i] != NULL) {
                freeThreads((*processes)[i]->threads, (*processes)[i]->threadAmt);
                free( (*processes)[i] );
            }
        }
        free(*processes);
    }
    free(processes);
}


// Frees a list of threads
void freeThreads( thread** threads, int threadAmt) {
    if(threads != NULL) {
        for( int i = 0; i < threadAmt; i++) {
            if(threads[i] != NULL) {
                freeBursts(threads[i]->bursts, threads[i]->burstNo);
                free( threads[i]);
            }
        }
        free(threads);
    }
}

// Frees a list of bursts
void freeBursts(cpuBurst** bursts, int burstAmt) {
    if(bursts != NULL) {
        for( int i = 0; i < burstAmt; i++) {
            if(bursts[i] != NULL) {
                free( bursts[i]);
            }
        }
        free(bursts);
    }
}

//TODO TEMP
heap* initializeHeap() {
    return NULL;
}

// Validates that new line is present at the end of the stdin line
bool validateLineEnding() {
    char tempchar;
    while( (tempchar = getc(stdin)) == ' '); // get rid of whitespace
    return (tempchar == '\n' || tempchar == '\0' || tempchar == EOF); // Checks final character for new line
}