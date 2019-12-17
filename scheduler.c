#include "headers.h"

struct process *PriotityQueue = NULL;
struct process *SRTNQueue = NULL;
bool isrunning = false;
bool parrived = false;
int RRQuantum;

void CleanResources(int signum);
void receivedfn(int signum); //sigchild handler
void RRQfn(int signum);      //fn to handle quantum expiring
void SRTNfn(int signum);

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

    int NumProcesses = 0;           //start with 0 processes
    struct process ReceivedProcess; //process placeholder
    struct process RunningProcess;  //process placeholder

    //initialize the msg queue to recv processes
    //from process generator
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    if (msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    if (AlgoUsed == HPF)
    {
        while (1)
        {
            signal(SIGCHLD, receivedfn); //empty handler create just to interrupt msgrcv
            //printf("RECEIVED1\n");
            int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
            if (rec_val == -1)
            {
                printf("At time %d Process %d finished\n", getClk(), RunningProcess.id);
                deleteProcessPQ(&PriotityQueue, RunningProcess);
                isrunning = false;
            }

            if (rec_val != -1)
            {
                printf("Time %d Process #%d received  \n", getClk(), ReceivedProcess.id);
                NumProcesses++;
                //printf("before PushPQ recv %d\n", ReceivedProcess.id);
                //PriotityQueue = &ReceivedProcess;
                PushPQ(&PriotityQueue, ReceivedProcess);
            }

            //printf("after PushPQ\n");
            if (!isrunning && !isEmptyPQ(&PriotityQueue))
            {
                printf("Starting a process\n");
                // printf("First Run\n");
                isrunning = true;
                //printf("queue %d\n", (*PriotityQueue).id);
                RunningProcess = *(PeekPQ(&PriotityQueue));
                printf("after PeekPQ %d\n", RunningProcess.id);
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
        signal(SIGUSR1, SRTNfn);     // empty handler for signal received by process generator when a process arrives
        signal(SIGCHLD, receivedfn); //empty handler create just to interrupt msgrcv
        while (1)
        {
            int sleepval = sleep(INT_MAX);
            sleepval = INT_MAX - sleepval-1;
            // printf("good morning \n");
            int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, IPC_NOWAIT);
            if (rec_val == -1 && isrunning)
            {
                printf("here 1 time %d\n", getClk());
                //increase wait time of all processes
                struct process *iterator = SRTNQueue;
                while (iterator != NULL)
                {
                    if (iterator->id != RunningProcess.id)
                    {
                        iterator->TimeWait += sleepval;
                    }
                    iterator = iterator->next;
                }
                //process finished
                printf("At time %d Process %d finished\n", getClk(), RunningProcess.id);
                deleteProcessSRTN(&SRTNQueue, RunningProcess);
                isrunning = false;
            }

            if (rec_val != -1)
            {
                printf("here 2 time %d\n", getClk());
                //  isrunning = false;
                //if not first time
                if (RunningProcess.id != 0)
                {
                    printf("here 2.5time %d\n", getClk());
                    RunningProcess.TimeRemaining -= sleepval;
                    printf("sleepval is %d and curren rem time is %d\n", sleepval, RunningProcess.TimeRemaining);
                    //decrease rem time of runnning process
                    struct process *iterator = SRTNQueue;
                    while (iterator != NULL)
                    {
                        if (iterator->id == RunningProcess.id)
                        {
                            iterator->TimeRemaining -= sleepval;
                        }
                        iterator = iterator->next;
                    }
                    if (ReceivedProcess.TimeRemaining < RunningProcess.TimeRemaining)
                    {
                        printf("here 2.8 time %d\n", getClk());

                        kill(RunningProcess.pid, SIGTSTP);
                        isrunning = false;
                        printf("At time %d Process %d Stopped\n", getClk(), RunningProcess.id);
                    }
                }

                NumProcesses++;
                PushSRTN(&SRTNQueue, ReceivedProcess);
                // printf("Time %d Process %d RECV\n", getClk(), ReceivedProcess.id);
            }

            if (!isrunning && !isEmptySRTN(&SRTNQueue))
            {
                printf("here 3 time %d\n", getClk());
                //  printf("Starting a process\n");
                isrunning = true;
                RunningProcess = *(PeekSRTN(&SRTNQueue));
                //printf("after RRQ dequeue %d\n", RunningProcess.id);

                //if first time running fork
                if (RunningProcess.pid == 900)
                {
                    printf("here 3.2 time %d\n", getClk());

                    printf("At time %d Process %d started\n", getClk(), RunningProcess.id);
                    int pid = fork();
                    RunningProcess.pid = pid;
                    PeekSRTN(&SRTNQueue)->pid = pid;
                    if (pid == 0)
                    {
                        //child
                        char temp[sizeof(int) * 4]; //up to 4 digits of input
                        sprintf(temp, "%d", RunningProcess.TimeRemaining);
                        char *argv[] = {"./process.out", temp, NULL};
                        execvp(argv[0], argv); //child executes process
                    }
                }
                else //continue it
                {
                    printf("here 3.5 time %d\n", getClk());

                    printf("At time %d Process %d resumed\n", getClk(), RunningProcess.id);
                    //  printf("will resume %d\n", RunningProcess.pid);
                    kill(RunningProcess.pid, SIGCONT);
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
    parrived = true;
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
    exit(0);
}