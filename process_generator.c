#include "headers.h"
#define HPF 0
#define SRTN 1
#define RR 2
int AlgoNum;
int RRQuantum;
void clearResources(int);

int main(int argc, char *argv[])
{
    //signal(SIGINT, clearResources);
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

    //!!!! cannot do char comparison like
    //string[0]!="b"
    // "?" Isn't a char but a string with just one char

    // '?' Is a char and should return true in s1[i] == '?'
    //atoi(string) =int

    char junk[256];
    char placeholder[2];
    char CompInit;

    //  fscanf(fileptr, "%c", &placeholder);
    // printf("%c\n", placeholder);
    // fscanf(fileptr, "%c", &placeholder);
    // printf("%c\n", placeholder);


    while (feof(fileptr)==0) //feof returns non-zero if EOF
    {
        fscanf(fileptr, "%c", &CompInit);
        if (CompInit == '#')
        {
            printf("FLUSHING\n");
            fgets(junk, 256, fileptr); //flush lline (comment with #)
        }
        else if (CompInit != ' ' && CompInit != '\t')
        {
            //sleep(1);
            printf("%c\n", CompInit);
        }

        // if (1) //EOf
        //     break;
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
    }

    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid == 0)
    {
        //child
        execvp("./clk.out", NULL);
    }
    else
    {
        //parent
    }
    pid = fork();
    if (pid == 0)
    {
        //child
        execvp("./scheduler.out", NULL);
    }
    else
    {
        //parent
    }
    printf("child exited\n");

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(false);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
