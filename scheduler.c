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
void CleanResources(int signum);
void remove_element(struct ProcessPCBEntry *array, int index, int array_length)
{
    int i;
    for (i = index; i < array_length - 1; i++)
        array[i] = array[i + 1];
}

void ResizeArr(struct ProcessPCBEntry *array, int ArrayLength)
{
    if (ArrayLength > 1)
    {
        printf("REALLOCINGGG length is %d\n", ArrayLength);
        array = realloc(array, (ArrayLength - 1) * sizeof(struct ProcessPCBEntry));
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, CleanResources);
    initClk();

    int NumProcesses = 0;          //start with 0 processes
    struct process CurrentProcess; //process placeholder
    struct ProcessPCBEntry *PCB;

    //initialize the msg queue to recv processes
    //from process generator
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    if (msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("scheduler msgqid = %d\n", msgqid);
    while (1)
    {

        int rec_val = msgrcv(msgqid, &CurrentProcess, sizeof(struct process), 0, !IPC_NOWAIT);

        if (rec_val == -1)
            perror("Error in receive");
        else
            printf("Time %d Process #%d received  \n", getClk(), CurrentProcess.id);

        NumProcesses++;

        //fork children upon arrival
        int pid = fork();
        if (pid == 0)
        {
            //child
            execvp("./process.out", NULL); //child executes process
        }
        int time = getClk();
        int stat;
        //init shared memory with process
        key_t shmid = shmget(pid, 4, IPC_CREAT | 0666);
        if ((long)shmid == -1)
        {
            perror("Error in creating shm!");
            exit(-1);
        }
        int *shmaddr = (int *)shmat(shmid, (void *)0, 0);
        if ((long)shmaddr == -1)
        {
            perror("Error in attaching the shm in process!");
            exit(-1);
        }
        (*shmaddr) = CurrentProcess.runningtime;

        waitpid(pid, 0, 0);
        printf("Process %d finished after %d\n", CurrentProcess.id, getClk() - time);
        
        //remove process from PCB
        if (NumProcesses == 1)
            PCB = (struct ProcessPCBEntry *)malloc(sizeof(struct process));
        else
        {
            ResizeArr(PCB, NumProcesses);
        }
        //init process in PCB
        // PCB[NumProcesses - 1].process = CurrentProcess;
        // PCB[NumProcesses - 1].pid = pid;
        // PCB[NumProcesses - 1].state = Running;
        // PCB[NumProcesses - 1].TimeExecution = 0;
        // PCB[NumProcesses - 1].TimeRemaining = CurrentProcess.runningtime;
        // PCB[NumProcesses - 1].TimeWait = 0;
        //  kill(childpid,SIGKILL);  //to kill(suspend child process)
    }

    //upon termination release the clock resources
    destroyClk(false);
}
void CleanResources(int signum)
{
    exit(0);
}