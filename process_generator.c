#include "headers.h"
int AlgoNum;
int RRQuantum;
void clearResources(int);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *fileptr = fopen("processes.txt", "r");
    if (fileptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    else
    {
        printf("File Opened\n");
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Enter 0 for HPF , 1 for SRTN , 2 for RR\n");
    int AlgoUsed;
    scanf("%d", &AlgoUsed);
    if (AlgoUsed == 0)
        AlgoNum = HPF;
    else if (AlgoUsed == 1)
        AlgoNum = SRTN;
    else if (AlgoUsed == 2)
    {
        AlgoNum == RR;
        printf("Enter Round Robin Quantum:\n");
        scanf("%d", &RRQuantum);
    }
    else
    {
        //invalid input exit
        //to do
        printf("Algo Num Invalid Process Generator will now exit\n ");
        killpg(getpgrp(), SIGKILL);
    }

    // 3. Initiate and create the scheduler and clock processes.
    int PidClk = fork();
    if (PidClk == 0)
    {
        //child
        execvp("./clk.out", NULL); //child executes clk process
    }


    int PidScheduler = fork();
    if (PidScheduler == 0)
    {
        if (AlgoUsed != RR)
        {
            char temp[sizeof(int) * 4];    //up to 4 digits of input
            sprintf(temp, "%d", AlgoUsed); //convert int to string
                                           //printf("algo used sprintf %s\n",temp);
            char *argv[] = {"./scheduler.out", temp, NULL};
            execvp("./scheduler.out", argv); //child executes scheduler process
        }
        else
        {
            //printf("the rrq is %d\n", RRQuantum);
            //pass RR quantum to scheduler
            char temp[sizeof(int) * 4];       //up to 4 digits of input
            sprintf(temp, "%d", AlgoUsed);    //convert int to string
            char tempRR[sizeof(int) * 4];     //up to 4 digits of input
            sprintf(tempRR, "%d", RRQuantum); //convert int to string
            char *argv[] = {"./scheduler.out", temp, tempRR, NULL};
            execvp("./scheduler.out", argv); //child executes scheduler process
        }
    }

    // sleep(5);
    // kill(PidScheduler, SIGCHLD);
    // sleep(5);
    // kill(PidScheduler, SIGCHLD);
    //sleep(INT_MAX);

    // 4. Use this function after creating the clock process to initialize clock
    //initClk();
    // To get time use this
    // int x = getClk();
    // printf("current time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

    //!!!! cannot do char comparison like
    //string[0]!="b"
    // "?" Isn't a char but a string with just one char

    // '?' Is a char and should return true in s1[i] == '?'
    //atoi(string) =int

    int Pdata;
    int NumProcesses = 0;
    int NumLines = 0;
    char junk[256];
    //count number of lines-1( number of process)
    while (fscanf(fileptr, "%*s %*s %*s %s", junk) == 1)
    {
        NumLines++;
    }
    NumProcesses = NumLines - 1;
    printf("We have %d Processes\n", NumProcesses);

    rewind(fileptr); //resets ptr to beginning of file
        //fscanf stop at space of end of line or EOF
    fscanf(fileptr, "%*s %*s %*s %*s"); // skips first comment line

    //initialize array of processes
    struct process *ProcessArr = (struct process *)malloc(sizeof(struct process) * NumProcesses);
    int i;
    for (i = 0; i < NumProcesses; i++)
    {
        //infile
        fscanf(fileptr, "%d", &Pdata); //read id
        ProcessArr[i].id = Pdata;
        fscanf(fileptr, "%d", &Pdata); //read arrival time
        ProcessArr[i].arrivaltime = Pdata;
        fscanf(fileptr, "%d", &Pdata); //read runtime
        ProcessArr[i].runningtime = Pdata;
        ProcessArr[i].TimeRemaining = Pdata;

        fscanf(fileptr, "%d", &Pdata); //read priority
        ProcessArr[i].priority = Pdata;
        //to be updated by scheduler
        ProcessArr[i].pid = 900;
        ProcessArr[i].state = Suspended;
        ProcessArr[i].TimeWait = 0;
        ProcessArr[i].TimeExecution = 0;
        ProcessArr[i].next = NULL;

        printf("process%d\t%d\t%d\t%d\t%d\n", i, ProcessArr[i].id, ProcessArr[i].arrivaltime, ProcessArr[i].runningtime, ProcessArr[i].priority);
    }

    // 6. Send the information to the scheduler at the appropriate time.
    key_t msgqid = msgget(MSGQKEY, 0666 | IPC_CREAT); // or msgget(12613, IPC_CREATE | 0644)
    if (msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    //printf("msgqid = %d\n", msgqid);

    initClk();
    int time = 0;
    while (1)
    {
        //sleep(1); //to synch with clk
        if (getClk() <= time)
        { //to wait for next clk cycle
            continue;
        }
        else
        {
            time = getClk();
        }

        // printf("time is %d \n", getClk());
        int j;
        struct process Process;
        for (j = 0; j < NumProcesses; j++)
        {
            if (ProcessArr[j].arrivaltime == time)
            {
                Process = ProcessArr[j];
                int send_val = msgsnd(msgqid, &Process, sizeof(struct process), !IPC_NOWAIT);
                if (send_val == -1)
                    perror("Errror in send");
                // else
                //  printf("time %d process #%d sent\n", time, Process.id);
            }
        }
    }

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    key_t msgqid = msgget(MSGQKEY, IPC_CREAT | 0666); // or msgget(12613, IPC_CREATE | 0644)
    //deleted msg queue between process generator and scheduler
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
