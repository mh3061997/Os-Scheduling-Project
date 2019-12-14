#include "headers.h"

struct process *PriotityQueue = NULL;

int HighestPriority = -1;
bool isrunning = false;
bool firstrun = true;
void CleanResources(int signum);
// void remove_element(struct ProcessPCBEntry *array, int index, int array_length)
// {
//     int i;
//     for (i = index; i < array_length - 1; i++)
//         array[i] = array[i + 1];
// }

// void ResizeArr(struct ProcessPCBEntry *array, int ArrayLength)
// {
//     // printf("reallocing length %d\n",ArrayLength);
//     // printf("size struct %d" ,sizeof(struct ProcessPCBEntry));
//     // printf("total new size %d\n", sizeof(struct ProcessPCBEntry) *ArrayLength);
//     if (ArrayLength > 1)
//     {
//         array = (struct ProcessPCBEntry *)realloc(array, sizeof(struct ProcessPCBEntry) * ArrayLength);
//     }
// }
void receivedfn(int signum);
int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();
    PriotityQueue = (struct process *)malloc(sizeof(struct process)*50);

    //PriotityQueue=NULL;
    //int AlgoUsed=atoi(argv[1]);
    printf("Algo used is %d\n", atoi(argv[1]));
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
    while (1)
    {
        signal(SIGCHLD, receivedfn);
        //printf("RECEIVED1\n");
        int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
        if (rec_val == -1)
        {
            printf("At time %d Process %d finished\n",getClk(),RunningProcess.id);
           // pop(&PriotityQueue);
            isrunning = false;
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
            //printf("before push recv %d\n", ReceivedProcess.id);
            //PriotityQueue = &ReceivedProcess;
            push(&PriotityQueue, ReceivedProcess);
        }

        //printf("after push\n");
        if (!isrunning && !isEmpty(&PriotityQueue))
        {
            printf("Starting a process\n");
            // printf("First Run\n");
            isrunning = true;
            //printf("queue %d\n", (*PriotityQueue).id);
            RunningProcess = *(peek(&PriotityQueue));
            printf("after peek %d\n", RunningProcess.id);
             pop(&PriotityQueue);
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