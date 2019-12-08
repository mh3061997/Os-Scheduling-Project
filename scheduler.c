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

int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();

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
    //printf("scheduler msgqid = %d\n", msgqid);
    //init PCB
    PCB = (struct ProcessPCBEntry *)malloc(sizeof(struct ProcessPCBEntry) * (NumProcesses + 1));
    bool isrunning = false;
    key_t shmid;
    int *shmaddr;
    int time;
    while (1)
    {

        int rec_val = msgrcv(msgqid, &ReceivedProcess, sizeof(struct process), 0, !IPC_NOWAIT);

        if (rec_val == -1)
            perror("Error in receive");
        else
            printf("Time %d Process #%d received  \n", getClk(), ReceivedProcess.id);

        NumProcesses++;
        if (NumProcesses != 1)
        {
            printf("Expanding PCB Numprocess %d\n", NumProcesses);
            ResizeArr(PCB, NumProcesses);
        }
        //init process in PCB
        PCB[NumProcesses - 1].process = ReceivedProcess;
        PCB[NumProcesses - 1].state = Running;
        PCB[NumProcesses - 1].TimeExecution = 0;
        PCB[NumProcesses - 1].TimeRemaining = ReceivedProcess.runningtime;
        PCB[NumProcesses - 1].TimeWait = 0;

        //search for process with highest priority
        int k;
        for (k = 0; k < NumProcesses; k++)
        {

            if (PCB[k].process.priority > HighestPriority)
            {
                HighestPriority = PCB[k].process.priority;
                NextProcess = PCB[k].process;
            }
        }

        //printf("Next Process #%d\n", ReceivedProcess.id);

        if (isrunning == false)
        {
            RunningProcess=NextProcess; //set next process in queue as running
            //fork children upon arrival
            isrunning == true;
            int pid = fork();
            PCB[NumProcesses - 1].pid = pid;

            if (pid == 0)
            {
                //child
                execvp("./process.out", NULL); //child executes process
            }
            time = getClk(); //to know how much time process took
            int stat;        //unused exit code

            //init shared memory with process
            shmid = shmget(pid, 4, IPC_CREAT | 0666);
            if ((long)shmid == -1)
            {
                perror("Error in creating shm!");
                exit(-1);
            }
            shmaddr = (int *)shmat(shmid, (void *)0, 0);
            if ((long)shmaddr == -1)
            {
                perror("Error in attaching the shm in process!");
                exit(-1);
            }
            (*shmaddr) = RunningProcess.runningtime;

            //should not wait bec. schedule must be available to recv processes if arrived
            //waitpid(pid, stat, 0); //Non Preemptive so wait
        }
        //if process finished
        int RemainingTimeForRunningProcess = (*shmaddr);
        printf("Remaining time %d\n", RemainingTimeForRunningProcess);
        if (RemainingTimeForRunningProcess < 1)
        {
            isrunning = false;
            int timetook = getClk() - time;
            printf("Process %d finished after %d\n", RunningProcess.id, timetook);
            NumProcesses--;
            //remove process from PCB
            if (NumProcesses > 1)
            {
                printf("Shrinking PCB Numprocess %d", NumProcesses);
                ResizeArr(PCB, NumProcesses);
            }
        }
        //  kill(childpid,SIGKILL);  //to kill(suspend child process)
    }

    //upon termination release the clock resources
    destroyClk(false);
}
void CleanResources(int signum)
{
    exit(0);
}