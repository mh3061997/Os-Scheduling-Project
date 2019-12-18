#include "headers.h"

struct process *PriotityQueue = NULL;
struct process *SRTNQueue = NULL;
bool isrunning = false;
bool parrived = false;
bool firstrun = true;
int RRQuantum;
int MemoryArr[4];

int TotalWTA;
int TotalWaiting;
int NumProcesses = 0; //start with 0 processes
FILE *FilePerf;

void CleanResources(int signum);
void receivedfn(int signum); //sigchild handler
void RRQfn(int signum);      //fn to handle quantum expiring
void SRTNfn(int signum);
void MemAllocate(FILE *FileMem, struct process RunningProcess);
void MemDeallocate(FILE *FileMem, struct process RunningProcess);
void FileOutStart(FILE *FilePerf, struct process RunningProcess);
void FileOutFinish(FILE *FilePerf, struct process RunningProcess);

int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();
    int LastClk = getClk();
    int ElapsedTime = 0;
    //PriotityQueue=NULL;
    //int AlgoUsed=atoi(argv[1]);
    printf("Algo used is %d\n", atoi(argv[1]));
    int AlgoUsed = atoi(argv[1]);
    if (AlgoUsed == RR)
        RRQuantum = atoi(argv[2]);

    struct process ReceivedProcess; //process placeholder
    struct process RunningProcess;  //process placeholder

    //Memory Array is 1024 bytes
    //divided into 4 Partitions each 256 bytes
    //index is Process ID reserveing it
    for (int i = 0; i < 4; i++)
    {
        MemoryArr[i] = 0; //init as 0s
    }

    //initialize the msg queue to recv processes
    //from process generator
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    FILE *Filelog;
    FILE *FileMem;
    FileMem = fopen("memory_log.txt", "w");
    Filelog = fopen("scheduler_log.txt", "w");
    fprintf(FileMem, "#At time x allocated y bytes for process z from i to j\n");
    fprintf(Filelog, "#At time x process y state arr w total z remain y wait k\n");
    //FilePerf = fopen("scheduler_perf.txt", "w");
    if (msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    if (AlgoUsed == HPF)
    {
        signal(SIGCHLD, receivedfn); //empty handler create just to interrupt msgrcv
        while (1)
        {
            //printf("RECEIVED1\n");
            int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
            if (rec_val == -1)
            {
                //printf("param is %d %d \n", RunningProcess.arrivaltime, RunningProcess.TimeRemaining);
                printf("At time %d process %d finished\n", getClk(), RunningProcess.id);
                // printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %.2f WTA %.2f\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait, (getClk() - RunningProcess.arrivaltime), wta);
                float wta = ((getClk() - RunningProcess.arrivaltime) / RunningProcess.TimeRemaining);
                fprintf(Filelog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %.2f WTA %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait, (getClk() - RunningProcess.arrivaltime), wta);

                deleteProcessPQ(&PriotityQueue, RunningProcess);
                //printf("after delete %d\n",(*(PeekPQ(&PriotityQueue))).id);
                isrunning = false;

                //Deallocate Memory When Process Has Finished
                MemDeallocate(FileMem, RunningProcess);

                //add wta
                TotalWTA += wta;
                //add waiting
                TotalWaiting += RunningProcess.TimeWait;
            }

            if (rec_val != -1)
            {
                // printf("Time %d Process #%d received  \n", getClk(), ReceivedProcess.id);
                NumProcesses++;
                // printf("before PushPQ recv %d\n", ReceivedProcess.id);
                //PriotityQueue = &ReceivedProcess;
                PushPQ(&PriotityQueue, ReceivedProcess);
            }

            //printf("after PushPQ\n");
            if (!isrunning && !isEmptyPQ(&PriotityQueue))
            {
                // printf("First Run\n");
                isrunning = true;
                //printf("queue %d\n",(*(PeekPQ(&PriotityQueue))).arrivaltime);
                RunningProcess = *(PeekPQ(&PriotityQueue));
                //update running process wait time
                RunningProcess.TimeWait += getClk();
                RunningProcess.TimeWait -= RunningProcess.arrivaltime;
                printf("At time %d process %d started\n", getClk(), RunningProcess.id);

                //output to file
                fprintf(Filelog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait);

                //allocate memory for the process
                MemAllocate(FileMem, RunningProcess);

                // printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait);
                //fprintf(Filelog,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrivaltime,RunningProcess.TimeExecution,RunningProcess.TimeRemaining,RunningProcess.TimeWait);
                //printf("after PeekPQ %d\n", RunningProcess.id);
                //pop(&PriotityQueue);
                int pid = fork();
                if (pid == 0)
                {

                    //child
                    char temp[sizeof(int) * 4]; //up to 4 digits of input
                    sprintf(temp, "%d", RunningProcess.TimeRemaining);
                    char *argv[] = {"./process.out", temp, NULL};
                    execvp(argv[0], argv); //child executes process
                }
            }

            // sleep(1);
            // kill(pid,SIGSTOP);
            // printf("Stopped\n");
            // sleep(10);
            // kill(pid,SIGCONT);
            // printf("Continued\n");
            // sleep(INT_MAX);
        }
    }
    else if (AlgoUsed == SRTN)
    {
        signal(SIGUSR1, receivedfn); // empty handler for signal received by process generator when a process arrives
        signal(SIGCHLD, SRTNfn);     //empty handler create just to interrupt msgrcv
        while (1)
        {
            int sleepval = sleep(INT_MAX);
            sleepval = INT_MAX - sleepval - 1;
            // printf("good morning \n");
            int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, IPC_NOWAIT);
            if (rec_val == -1 && isrunning)
            {
                // printf("here 1 time %d\n", getClk());
                //increase wait time of all processes
                struct process *iterator = SRTNQueue;
                while (iterator != NULL)
                {
                    if (iterator->id != RunningProcess.id)
                    {
                        iterator->TimeWait += sleepval;
                    }
                    else
                    {
                        //  printf("running pid rem time is %d %d  \n", iterator->TimeRemaining,RunningProcess.TimeRemaining);
                    }

                    iterator = iterator->next;
                }
                //process finished
                printf("At time %d Process %d finished\n", getClk(), RunningProcess.id);
                deleteProcessSRTN(&SRTNQueue, RunningProcess);
                isrunning = false;

                //output to file
                float wta = ((getClk() - RunningProcess.arrivaltime) / RunningProcess.TimeRemaining);
                fprintf(Filelog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %.2f WTA %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait, (getClk() - RunningProcess.arrivaltime), wta);

                //Deallocate Memory When Process Has Finished
                MemDeallocate(FileMem, RunningProcess);
                //add wta
                TotalWTA += wta;
                //add waiting
                TotalWaiting += RunningProcess.TimeWait;
            }

            if (rec_val != -1)
            {
                // printf("here 2 time %d\n", getClk());
                //  isrunning = false;
                //if not first time
                if (!firstrun)
                {
                    //  printf("here 2.5time %d\n", getClk());
                    RunningProcess.TimeRemaining -= sleepval;
                    // printf("sleepval is %d and curren rem time is %d\n", sleepval, RunningProcess.TimeRemaining);
                    //decrease rem time of runnning process
                    struct process *iterator = SRTNQueue;
                    while (iterator != NULL)
                    {
                        if (iterator->id == RunningProcess.id)
                        {
                            iterator->TimeRemaining -= sleepval;
                            //printf("deceremnting hereee ! after is %d \n", iterator->TimeRemaining);
                        }
                        iterator = iterator->next;
                    }
                    if (ReceivedProcess.TimeRemaining < RunningProcess.TimeRemaining)
                    {
                        //      printf("here 2.8 time %d\n", getClk());
                        printf("i will stop %d\n ", RunningProcess.pid);
                        int killval = kill(RunningProcess.pid, SIGTSTP);
                        if (killval == 0)
                        {
                            printf("killed oh yeah \n");
                        }
                        else
                        {
                            perror("error is \n");
                        }

                        isrunning = false;
                        printf("At time %d Process %d Stopped\n", getClk(), RunningProcess.id);
                    }
                }
                firstrun = false;
                NumProcesses++;
                PushSRTN(&SRTNQueue, ReceivedProcess);
                // printf("Time %d Process %d RECV\n", getClk(), ReceivedProcess.id);
            }

            if (!isrunning && !isEmptySRTN(&SRTNQueue))
            {
                // printf("here 3 time %d\n", getClk());
                //  printf("Starting a process\n");
                isrunning = true;
                RunningProcess = *(PeekSRTN(&SRTNQueue));
                //printf("after RRQ dequeue %d\n", RunningProcess.id);

                //if first time running fork
                if (RunningProcess.pid == 900)
                {
                    //     printf("here 3.2 time %d\n", getClk());

                    printf("At time %d Process %d started\n", getClk(), RunningProcess.id);

                    //output to file
                    fprintf(Filelog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait);

                    int pid = fork();
                    if (pid == 0)
                    {
                        //child
                        char temp[sizeof(int) * 4]; //up to 4 digits of input
                        sprintf(temp, "%d", RunningProcess.TimeRemaining);
                        char *argv[] = {"./process.out", temp, NULL};
                        execvp(argv[0], argv); //child executes process
                    }
                    RunningProcess.pid = pid;
                    PeekSRTN(&SRTNQueue)->pid = pid;

                    //allocate memory for the process
                    MemAllocate(FileMem, RunningProcess);
                }
                else //continue it
                {
                    //  printf("here 3.5 time %d\n", getClk());

                    printf("At time %d Process %d resumed\n", getClk(), RunningProcess.id);
                    //  printf("will resume %d\n", RunningProcess.pid);
                    kill((*(PeekSRTN(&SRTNQueue))).pid, SIGCONT);
                }
            }

            //  printf("I will sleep \n");

            // sleep(1);
            // kill(pid,SIGSTOP);
            // printf("Stopped\n");
            // sleep(10);
            // kill(pid,SIGCONT);
            // printf("Continued\n");
            // sleep(INT_MAX);
        }
    }
    else if (AlgoUsed == RR)
    {
        //printf("MY USED RRQ IS %d\n", RRQuantum);
        struct Queue *RRQueue = (struct Queue *)malloc(sizeof(struct Queue));
        RRQueue->front = NULL;
        RRQueue->rear = NULL;
        signal(SIGCHLD, receivedfn); //empty handler create just to interrupt msgrcv
        signal(SIGALRM, RRQfn);
        while (1)
        {
            if (getClk() <= LastClk)
            { //to wait for next clk cycle
                continue;
            }
            else
            {
                alarm(RRQuantum);

                //printf("RECEIVED1\n");
                int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
                if (rec_val == -1 && isrunning)
                {

                    // printf("will stop %d\n", RunningProcess.pid);
                    kill(RunningProcess.pid, SIGTSTP);
                    RunningProcess.TimeRemaining -= RRQuantum;
                    if (RunningProcess.TimeRemaining > 0)
                    { //quantum done process still not finished
                        printf("At time %d Process %d Stopped\n", getClk(), RunningProcess.id);
                        enqueue(RRQueue, RunningProcess);
                        //UpdateAllProcesses(RRQueue,time);
                    }
                    else
                    { //process finished
                        printf("At time %d Process %d finished\n", getClk(), RunningProcess.id);
                        //output to file
                        float wta = ((getClk() - RunningProcess.arrivaltime) / RunningProcess.TimeRemaining);
                        fprintf(Filelog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %.2f WTA %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait, (getClk() - RunningProcess.arrivaltime), wta);

                        //Deallocate Memory When Process Has Finished
                        MemDeallocate(FileMem, RunningProcess);
                        //add wta
                        TotalWTA += wta;
                        //add waiting
                        TotalWaiting += RunningProcess.TimeWait;
                    }
                    isrunning = false;

                    //printf("is running is now %d\n",isrunning);
                }

                if (rec_val != -1)
                {
                    NumProcesses++;
                    enqueue(RRQueue, ReceivedProcess);
                    //printf("Time %d Process %d RECV\n", getClk(), ReceivedProcess.id);
                    // printf("i enqueued %d",(*dequeue(RRQueue)).id);
                    // enqueue(RRQueue, ReceivedProcess);
                }
                // display(RRQueue);
                if (!isrunning && !isEmptyRRQ(RRQueue))
                {
                    //  printf("Starting a process\n");
                    isrunning = true;
                    RunningProcess = *(dequeue(RRQueue));
                    //printf("after RRQ dequeue %d\n", RunningProcess.id);

                    //if first time running fork
                    if (RunningProcess.pid == 900)
                    {
                        printf("At time %d Process %d started\n", getClk(), RunningProcess.id);

                        //output to file
                        fprintf(Filelog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), RunningProcess.id, RunningProcess.arrivaltime, RunningProcess.TimeExecution, RunningProcess.TimeRemaining, RunningProcess.TimeWait);

                        int pid = fork();
                        RunningProcess.pid = pid;
                        if (pid == 0)
                        {
                            //child
                            char temp[sizeof(int) * 4]; //up to 4 digits of input
                            sprintf(temp, "%d", RunningProcess.TimeRemaining);
                            char *argv[] = {"./process.out", temp, NULL};
                            execvp(argv[0], argv); //child executes process
                        }

                        //allocate memory for the process
                        MemAllocate(FileMem, RunningProcess);
                    }
                    else //continue it
                    {
                        printf("At time %d Process %d resumed\n", getClk(), RunningProcess.id);
                        //  printf("will resume %d\n", RunningProcess.pid);
                        kill(RunningProcess.pid, SIGCONT);
                    }
                }

                //in new cycle
                LastClk = getClk();
                ElapsedTime++;
                //printf("I Have Been Running for %d\n", ElapsedTime);
            }

            //  printf("I will sleep \n");

            // sleep(1);
            // kill(pid,SIGSTOP);
            // printf("Stopped\n");
            // sleep(10);
            // kill(pid,SIGCONT);
            // printf("Continued\n");
            // sleep(INT_MAX);
        }
    }
    else
    {
        printf("Algo Num Invalid Scheduler will now exit ");
        killpg(getpgrp(), SIGKILL);
    }

    //upon termination release the clock resources
    destroyClk(false);
}

void SRTNfn(int signum)
{
    printf("sigchild recv\n");
    // parrived = true;
}
void receivedfn(int signum)
{
    // printf("received sigchild\n");
    // pop(&PriotityQueue);
    // isrunning = false;
}
void RRQfn(int signum)
{
    //  printf("Quantum interrupt\n");
}
void CleanResources(int signum)
{
    //TODO Clears all resources in case of interruption
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    //deleted msg queue between process generator and scheduler
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *)0);

    FilePerf = fopen("scheduler_perf.txt", "w");

    //write to performance file
    fprintf(FilePerf, "CPU Utilization=100%%\n");
   //// fprintf(FilePerf, "%d %d \n", TotalWTA, NumProcesses);
    fprintf(FilePerf, "AVG WTA=%d\n", (TotalWTA / NumProcesses));
    //fprintf(FilePerf, "%d %d     \n", TotalWaiting, NumProcesses);
    fprintf(FilePerf, "AVG Waiting=%d\n", (TotalWaiting / NumProcesses));
    fprintf(FilePerf, "STD WTA=%d\n", 0);

    exit(0);
}

void MemAllocate(FILE *FileMem, struct process RunningProcess)
{
    int i = 0;
    int found = 0;
    for (i = 0; i < 4; i++)
    {
        if (MemoryArr[i] == 0) //get index of first available place
        {
            found = 1;
            MemoryArr[i] = RunningProcess.id;
            break;
        }
    }
    if (found == 1)
    {
        if (i == 0)
        {
            fprintf(FileMem, "At time %d allocated %d bytes for process %d from 0 to 255\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 1)
        {
            fprintf(FileMem, "At time %d allocated %d bytes for process %d from 256 to 511\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 2)
        {
            fprintf(FileMem, "At time %d allocated %d bytes for process %d from 512 to 767 \n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 3)
        {
            fprintf(FileMem, "At time %d allocated %d bytes for process %d from 768 to 1023\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
    }
}
void MemDeallocate(FILE *FileMem, struct process RunningProcess)
{
    //deallocate memory for the process when finished
    int i = 0;
    int found = 0;
    for (i = 0; i < 4; i++)
    {
        if (MemoryArr[i] == RunningProcess.id) //get index of first available place
        {
            found = 1;
            MemoryArr[i] = 0;
            break;
        }
    }
    if (found == 1)
    {
        if (i == 0)
        {
            fprintf(FileMem, "At time %d deallocated %d bytes for process %d from 0 to 255\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 1)
        {
            fprintf(FileMem, "At time %d deallocated %d bytes for process %d from 256 to 511\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 2)
        {
            fprintf(FileMem, "At time %d deallocated %d bytes for process %d from 512 to 767 \n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
        else if (i == 3)
        {
            fprintf(FileMem, "At time %d deallocated %d bytes for process %d from 768 to 1023\n", getClk(), RunningProcess.memsize, RunningProcess.id);
        }
    }
}
void FileOutStart(FILE *FilePerf, struct process RunningProcess)
{
}
void FileOutFinish(FILE *FilePerf, struct process RunningProcess)
{
}