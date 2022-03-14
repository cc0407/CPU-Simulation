#include "main.h"

bool detailed = false;
bool verbose = false;
int RRTime = 0;
int timeElapsed = 0;
char* enumString[5] = {"new", "ready", "running", "blocked", "terminated"};
int threadSwitch;
int processSwitch;

int main(int argc, char* argv[]) {
    process*** processes;
    int processAmt;
    int CPUUtilizationTime = 0;
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
        if(strcmp(argv[i], "-d") == 0) // -d flag was presented
            detailed = true;
        if(strcmp(argv[i], "-v") == 0) // -d flag was presented
            verbose = true;
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
    //printHeap(h);

    node* currNode;
    thread* currThread;
    int cpuCount = -1;
    int prevPNo = -1;
    int contextCount = 0;
    while( !isEmpty(h) ) {
        updateReadyQueue(h, timeElapsed); // See if there are any new processes
        if(cpuCount >= 0) { // CPU is busy with a process for [cpuCount] time units
            if(cpuCount == 0) {
                if(currThread->finTime == -2) // End of burst and no more bursts left in this thread
                stateSwitch(currThread, TERMINATED);
                currThread->finTime = timeElapsed;
            }
            else {
                timeElapsed++;
                CPUUtilizationTime++;
            }
            cpuCount--;
            continue;
        }

        if(timeElapsed >= minKey(h)) { // Next thread is available
            currNode = removeMin(h);
            currThread = (thread*)(currNode->data);

            if( prevPNo == -1)// Update process number to be this process
                prevPNo = currThread->PNo;
            else if( prevPNo == currThread->PNo ) // switch context to 
                contextCount = threadSwitch;
            else
                contextCount = processSwitch;

            while(contextCount > 0) {
                updateReadyQueue(h, timeElapsed);
                timeElapsed++;
                contextCount--;
            }

            switch (currThread->s) {
                case BLOCKED:
                case READY:
                    stateSwitch(currThread, RUNNING);
                    /* FCFS */
                    cpuCount = consumeTime(h, currNode, -1);// -1 because FCFS
                    break;
                case RUNNING: // For RR
                    break;
                default:
                    break;
            }
            
            free(currNode);
        }

    }

    if(RRTime == 0)
        printf("FCFS Scheduling\n");
    else
        printf("Round Robin Scheduling (quantum = %d time units)\n", RRTime);

    printf("Total Time required is %d units\n", timeElapsed);
    printf("Average Turnaround Time is %.1f time units\n", getAverageTurnaroundTime(*processes, processAmt));
    if(timeElapsed != 0)
        printf("CPU Utilization is %d%%\n", CPUUtilizationTime / ((float)timeElapsed) * 100);
    else
        printf("CPU Utilization is 0%%\n");

    if( detailed ) {
        printProcesses(*processes, processAmt);
        //printHeap(h); //TODO DEBUGGING
    }
    freeProcesses(processes, processAmt);
    freeHeap(h);
    return 0;
}

int consumeTime(heap* h, node* n, int amt) {
    
    //FCFS
    thread* t = (thread*)n->data;
    cpuBurst** b = t->bursts;
    int bNo = n->currBurst;
    int num = b[bNo]->currCpuTime; // Pull the CPU time from the current burst

    if(num == 0) { // cpu time for this burst has already been consumed
        stateSwitch(t, BLOCKED);
        n->currBurst++;
        insertItem(h, timeElapsed + b[bNo]->currIoTime, t, bNo); // consume iotime for this burst
    }
    else { // consume cpu time for this burst
        b[bNo]->currCpuTime = 0;
        if(b[bNo]->ioTime == -1) { // Final burst, no io time
            t->finTime = -2;
        }
        else {
            insertItem(h, b[bNo]->currIoTime, t, bNo); // consume burst and add back into queue
        }
    }
    return num;
}

heap* initializePriorityQueue(process*** p, int* processAmt, int* threadSwitch, int* processSwitch) {
    if( scanf(" %d %d %d", processAmt, threadSwitch) != 3 || !validateLineEnding()) {
        fprintf(stderr, "Error ingesting line 0 of input file.\n");
        return NULL;
    }
    //TODO DEBUGGING
    //printf("processAmt: %d\nInternal Time: %d\nExternal Time: %d\n", *processAmt, *threadSwitch, *processSwitch);

    /* Create process array */
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

    /* Copy pointers to process array into heap */
    heap* h = initializeHeap(*p, pNum);

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
        newBurst->currCpuTime = cpuTime;
        newBurst->currIoTime = ioTime;

        
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
    newBurst->ioTime = -1;
    newBurst->currCpuTime = cpuTime;
    newBurst->currIoTime = ioTime;

    return bList;
}

void updateReadyQueue(heap* h, int timeElapsed) {
    thread* t;
    if(h != NULL) { // Null Check for heap
        for(int i = 0; i < h->curr_size; i++) {
            t = (thread*)(h->harr)[i]->data;
            if(t->arrTime <= timeElapsed && t->s == NEW) { // Process should be added to the ready queue
                if(verbose) {
                    printf("At time %d: Thread %d of Process %d moves from new to ready.\n", timeElapsed, t->TNo, t->PNo);
                }
                t->s = READY;
            }
        }
    }
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
                    getTotalServiceTime(currThread), getTotalIOTime(currThread), getTurnaroundTime(currThread), currThread->finTime);
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
        for(int i = 0; i < t->burstNo - 1; i++) {
            sum+= t->bursts[i]->ioTime;
        }
        return sum;
    }
    else
        return sum;
}

int getTotalServiceTime(thread* t) {
    int sum = 0;
    if ( t != NULL ) {
        for(int i = 0; i < t->burstNo; i++) {
            sum+= t->bursts[i]->cpuTime;
        }
        return sum;
    }
    else
        return sum;
}

int getTurnaroundTime(thread* t) {
    if ( t != NULL ) {
        return t->finTime - t->arrTime;
    }
    return -1;
}

float getAverageTurnaroundTime(process** processes, int processAmt) {
    float avgTime = 0;
    float threadAvgTime;
    if(processes != NULL) {
        for(int i = 0; i < processAmt; i++) { // Iterate through each process
            threadAvgTime = 0;
            for(int j = 0; j < processes[i]->threadAmt; j++) { // Iterate through each thread
                threadAvgTime += getTurnaroundTime(processes[i]->threads[i]);
            }
            threadAvgTime /= processes[i]->threadAmt;
            avgTime += threadAvgTime;
        }
    }
    return avgTime;
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

// Validates that new line is present at the end of the stdin line
bool validateLineEnding() {
    char tempchar;
    while( (tempchar = getc(stdin)) == ' '); // get rid of whitespace
    return (tempchar == '\n' || tempchar == '\0' || tempchar == EOF); // Checks final character for new line
}

/* HEAP FUNCTIONS */
bool isEmpty(heap* h) {
    if(h == NULL) {
        return true;
    }
    return h->curr_size == 0;
}

//TODO TEMP
heap* initializeHeap(process** pList, int pNum) {
    // Create a new heap
    heap* h;
    h = (heap*)malloc(sizeof(heap));
    h->curr_size = 0;

    // Ingest data from array into heap
    for(int i = 0; i < pNum; i++) {
        for(int j = 0; j < pList[i]->threadAmt; j++) {
            insertItem(h, pList[i]->threads[j]->arrTime, pList[i]->threads[j], 0);
        }
    }

    return h;
}

void insertItem(heap* h, int key, void* data, int currBurst) {
    // Create new node
    node* newNode = (node*)malloc(sizeof(node));
    newNode->currBurst = currBurst;
    newNode->key = key;
    newNode->data = data;

    // Add node to heap
    h->curr_size++;
    if(h->curr_size <= 1) { // no nodes in heap yet
        h->harr = (node**)malloc(h->curr_size * sizeof(node*));
        (h->harr)[0] = newNode;
    }
    else { // Nodes present in heap
        h->harr = (node**)realloc(h->harr, h->curr_size * sizeof(node*)); // Make space for another node
        //bubble(h);  // Move all nodes one to the right
        (h->harr)[h->curr_size-1] = newNode; // Set last node to new node
        upheap(h, h->curr_size-1); // Restore heap balance
    }
}

// Returned value must be freed by the caller
node* removeMin(heap* h) {
    node* topNode;
    if(h != NULL && h->curr_size > 0) {
        topNode = minElement(h);
        (h->harr)[0] = NULL; // Remove the pointer from the heap
        swapNodes( &((h->harr)[0]), &((h->harr)[h->curr_size-1]) ); // Swap root node with last node

        h->curr_size--; // Reallocate heap
        h->harr = (node**)realloc(h->harr, h->curr_size * sizeof(node*));

        downheap(h, 0); // Rebalance heap
    }

    return topNode;
    
}

void upheap(heap* h, int i) {
    thread* t1;
    thread* t2;
    
    int currIndex;
    int parentIndex = getParentIndex(i);
    if( (h->harr)[parentIndex]->key > (h->harr)[i]->key ) { // left node is greater than current node
        swapNodes( &((h->harr)[parentIndex]), &((h->harr)[i]) );
        upheap(h, parentIndex);
    }
    else if( (h->harr)[parentIndex]->key == (h->harr)[i]->key ) { // left node is equal to right node TODO NOT SURE IF THIS IS RIGHT
        t1 = (thread*) (h->harr)[parentIndex]->data;
        t2 = (thread*) (h->harr)[i]->data;

        if( t1->PNo > t2->PNo ) { // Process number of left node is greater than right node
            swapNodes( &((h->harr)[parentIndex]), &((h->harr)[i]) );
            upheap(h, parentIndex);
        }
    }
}

void downheap(heap* h, int i) {
    node* n1;
    node* n2;
    thread* t1;
    thread* t2;
    
    int min;
    int rightIndex= getRightIndex(i);
    int leftIndex = getLeftIndex(i);

    if(h->curr_size <= 1) {
        return;
    }

    // Bound checking
    if(leftIndex < 0 || leftIndex >= h->curr_size) { leftIndex = -1;}
    if(rightIndex < 0 || rightIndex >= h->curr_size) { rightIndex = -1;}

    min = i; // Set min to parent
    if(leftIndex != -1 && h->harr[leftIndex]->key < h->harr[min]->key) { // Compare left with min
        min = leftIndex;
    }
    if(rightIndex != -1 && h->harr[rightIndex]->key < h->harr[min]->key) { // Compare right with min
        min = rightIndex;
    }
    if(min != i) { // Swap the minimum with i
        swapNodes( &((h->harr)[min]), &((h->harr)[i]) );
        downheap(h, min);
    }
}

void swapNodes(node** n1, node** n2) {
    node* temp;
    temp = *n1;
    *n1 = *n2;
    *n2 = temp;
}

void printHeap(heap* h) {
    thread* t;
    if(h != NULL) {
        for(int i = 0; i < h->curr_size; i++) {
            t = (thread*)(h->harr)[i]->data;
            printf("Key: %d, Thread #: %d, Process #: %d, currentBurst: %d\n", 
            (h->harr)[i]->key, t->TNo, t->PNo, (h->harr)[i]->currBurst);
        }
    }
}

void freeHeap(heap* h) {
    if(h != NULL) {
        for(int i = 0; i < h->curr_size; i++) {
            free((h->harr)[i]);
        }
        if(h->harr != NULL)
            free(h->harr);
        free(h);
    }
}

int minKey(heap* h) {
    if(h != NULL && h->curr_size != 0) {
        return ( (h->harr)[0]->key);
    }
    return -1;
}

node* minElement(heap* h) {
    if(h != NULL && h->curr_size != 0) {
        return (h->harr)[0];
    }
    return NULL;
}

int getParentIndex(int index) {
    return (index - 1) / 2;
}

int getLeftIndex(int index) {
    return 2*index + 1;
}

int getRightIndex(int index) {
    return 2*index + 2;
}

int min( int n1, int n2 ) {
    return (n1 <= n2) ? n1 : n2;
}

void stateSwitch(thread* t, state s) {
    if(t != NULL) {
        state prevState = t->s;
        t->s = s;
        if(verbose)
            printf("At time %d: Thread %d of Process %d moves from %s to %s.\n", timeElapsed, t->TNo, t->PNo, enumString[prevState], enumString[s]);
    }
}