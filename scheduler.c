#include "headers.h"

struct process *PriotityQueue = NULL;
bool isrunning = false;
int RRQuantum;
void CleanResources(int signum);
void receivedfn(int signum); //sigchild handler

int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();

    //PriotityQueue=NULL;
    //int AlgoUsed=atoi(argv[1]);
    printf("Algo used is %d\n", atoi(argv[1]));
    int AlgoUsed = atoi(argv[1]);
   if(AlgoUsed==RR)
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
    }
    else if (AlgoUsed == RR)
    {
        //printf("MY USED RRQ IS %d\n", RRQuantum);
        struct Queue *RRQueue = (struct Queue *)malloc(sizeof(struct Queue));
        RRQueue->front = NULL;
        RRQueue->rear = NULL;
        while (1)
        {
            signal(SIGCHLD, receivedfn); //empty handler create just to interrupt msgrcv
            //printf("RECEIVED1\n");

            int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
            if (rec_val == -1)
            {
                if (RunningProcess.TimeRemaining > 0)
                { //quantum done process still not finished
                    enqueue(RRQueue, RunningProcess);
                    //UpdateAllProcesses(RRQueue,time);
                }
                else
                { //process finished
                    printf("At time %d Process %d finished\n", getClk(), RunningProcess.id);
                    isrunning = false;
                }
            }

            if (rec_val != -1)
            {
                // if (firstrun)
                // {
                //     printf("Initializing queue\n");
                //     firstrun = false;
                //     PriotityQueue = newprocess(ReceivedProcess);
                // }
                printf("Time %d Process #%d received  \n", getClk(), ReceivedProcess.id);
                NumProcesses++;
                //printf("before PushPQ recv %d\n", ReceivedProcess.id);
                //PriotityQueue = &ReceivedProcess;
                enqueue(RRQueue, ReceivedProcess);
            }

            //printf("after PushPQ\n");
            if (!isrunning && !isEmptyRRQ(PriotityQueue))
            {
                printf("Starting a process\n");
                isrunning = true;
                RunningProcess = *(dequeue(RRQueue));
                printf("after RRQ dequeue %d\n", RunningProcess.id);

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
    else
    {
        printf("Algo Num Invalid Scheduler will now exit ");
        kill(getpgrp(), SIGKILL);
    }

    //upon termination release the clock resources
    destroyClk(false);
}
void receivedfn(int signum)
{
    // printf("received sigchild\n");
    // pop(&PriotityQueue);
    // isrunning = false;
}
void CleanResources(int signum)
{
    //TODO Clears all resources in case of interruption
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    //deleted msg queue between process generator and scheduler
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}