#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    int NumProcesses = 0;   //start with 0 processes
    struct process Process; //process placeholder
    //initialize the msg queue to recv processes
    //from process generator
    key_t msgqid = msgget(MSGQKEY, 0666 |IPC_CREAT); // or msgget(12613, IPC_CREATE | 0644)
    if (msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("scheduler msgqid = %d\n", msgqid);
    while (1)
    {
            printf("scheduler here = %d\n", msgqid);

        int rec_val = msgrcv(msgqid, &Process, sizeof(struct process), 0, !IPC_NOWAIT);

        if (rec_val == -1)
            perror("Error in receive");
        else
            printf("Process #%d received \n", Process.id);
    }

    //upon termination release the clock resources
    destroyClk(false);
}
