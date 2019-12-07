#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
void CleanResources(int signum);

int main(int agrc, char *argv[])
{
    signal(SIGINT,CleanResources);
    initClk();
    printf("I am process %d\n",getpid());
    //TODO it needs to get the remaining time from somewhere
    //will save remaining time in memory with id of it's pid
    /* initialize shared memory */
    key_t shmid = shmget(getpid(), 4, IPC_CREAT | 0644);
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
    remainingtime = (*shmaddr);

    int time = getClk();
    while (remainingtime > 0)
    {
        remainingtime =(*shmaddr) ;
        if (getClk() <= time)
        { //to wait for next clk cycle
            continue;
        }
        else
        {
            time = getClk();
        }
        (*shmaddr)--;
    }

    destroyClk(false);

    return 0;
}

void CleanResources(int signum){
exit(0);
}