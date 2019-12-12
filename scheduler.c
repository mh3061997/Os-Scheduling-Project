#include "headers.h"

#define Suspended 0
#define Running 1

struct ProcessPCBEntry
{
    struct process process;
    int pid;
    int state;
    int TimeWait;
    int TimeExecution;
    int TimeRemaining;
    
};
int HighestPriority = -1;
void CleanResources(int signum);
void remove_element(struct ProcessPCBEntry *array, int index, int array_length)
{
    int i;
    for (i = index; i < array_length - 1; i++)
        array[i] = array[i + 1];
}

void ResizeArr(struct ProcessPCBEntry *array, int ArrayLength)
{
    // printf("reallocing length %d\n",ArrayLength);
    // printf("size struct %d" ,sizeof(struct ProcessPCBEntry));
    // printf("total new size %d\n", sizeof(struct ProcessPCBEntry) *ArrayLength);
    if (ArrayLength > 1)
    {
        array = (struct ProcessPCBEntry *)realloc(array, sizeof(struct ProcessPCBEntry) * ArrayLength);
    }
}
void receivedfn(int signum);
int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();
    //int AlgoUsed=atoi(argv[1]);
    printf("Algo used is %d\n",atoi(argv[1]));
    int NumProcesses = 0;          //start with 0 processes
    struct process ReceivedProcess; //process placeholder
    struct process RunningProcess; //process placeholder
    struct process NextProcess;
    struct ProcessPCBEntry *PCB;

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
        // signal(SIGCHLD,receivedfn);
        printf("RECEIVED1\n");
        int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);
        if (rec_val == -1)
            perror("Error in receive");
        else
            printf("Time %d Process #%d received  \n", getClk(), ReceivedProcess.id);

        NumProcesses++;
                 printf("RECEIVED2\n");
                  
            int pid=fork();
             if (pid == 0)
            {
                //child
                 char temp[sizeof(int)*4]; //up to 4 digits of input 
                 sprintf(temp,"%d",5);
                char* argv[]={"./process.out",temp,NULL};
                execvp(argv[0],argv); //child executes process
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
void receivedfn(int signum){
    printf("received sigchild\n");
}
void CleanResources(int signum)
{
     //TODO Clears all resources in case of interruption
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    //deleted msg queue between process generator and scheduler
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}