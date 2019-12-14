#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
//added by me
#include <string.h>
#include <time.h>
///////

#define Suspended 0
#define Running 1

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define SHMProGenSched 350
#define MSGQKEY 200

//added by me//////////////////
struct process
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int pid;
    int state;
    int TimeWait;
    int TimeExecution;
    int TimeRemaining;
    struct process *next;
};

//HPF Priority queue functions

// Function to Create A New struct process
struct process *newprocess(struct process p)
{
    struct process *temp = (struct process *)malloc(sizeof(struct process));
    temp->arrivaltime = p.arrivaltime;
    temp->priority = p.priority;
    temp->runningtime = p.runningtime;
    temp->id = p.id;
    temp->pid = p.pid;
    temp->state = p.state;
    temp->TimeWait = p.TimeWait;
    temp->TimeExecution = p.TimeExecution;
    temp->TimeRemaining = p.TimeRemaining;
    temp->next = NULL;

    return temp;
}

// Return the value at head
struct process *peek(struct process **head)
{
    return (*head);
}

// Removes the element with the
// highest priority from the list
void pop(struct process **head)
{
    struct process *temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

void deleteProcess(struct process **head, struct process p)
{

    if ((*head)->id == p.id)
    { //if head is going to be deletedd
        pop(head);
    }
    else
    {
        struct process *iter = (*head);
        while (iter->next != NULL)
        {
            if (iter->id == p.id)
            {
                iter = iter->next;
            }
            iter = iter->next;
        }
    }
}
// Function to push according to priority
void push(struct process **head, struct process p)
{
    struct process *start = (*head);

    // Create new struct process
    struct process *temp = newprocess(p);
    printf("Pushing id %d\n",temp->id);

    // Special Case: The head of list has lesser
    // priority than new struct process. So insert new
    // struct process before head struct process and change head struct process.
    if ((*head)->priority > p.priority)
    {

        // Insert New struct process before head
        temp->next = *head;
        (*head) = temp;
    }
    else
    {

        // Traverse the list and find a
        // position to insert new struct process
        while (start->next != NULL &&
               start->next->priority < p.priority)
        {
            start = start->next;
        }

        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;
    }
}

int isEmpty(struct process** head) 
{ 
	return (*head) == NULL; 
} 
//
//////////////////////////

///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{

    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
