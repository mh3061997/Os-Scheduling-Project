#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
void CleanResources(int signum);

int main(int agrc, char *argv[])
{
  signal(SIGINT, CleanResources);
  initClk();
  int TimeStart = getClk();
  int LastClk = TimeStart;
  int ElapsedTime = 0;
  printf("I am process %d\n", getpid());
  //TODO it needs to get the remaining time from somewhere
  //will save remaining time in memory with id of it's pid
  /* initialize shared memory */
  remainingtime = atoi(argv[1]);
  printf("my rem time is %d\n", remainingtime);
int temp=0;
  while (ElapsedTime < remainingtime)
  {

    if (getClk() <= LastClk)
    { //to wait for next clk cycle
      continue;
    }
    else
    {
      //in new cycle
      LastClk=getClk();
      ElapsedTime++;
      //printf("I Have Been Running for %d\n", ElapsedTime);
      
    }
  }
  printf("I Executed for %d\n",ElapsedTime);
  printf("I Finished After %d\n", getClk()-TimeStart);
  signal(getppid(),SIGCHLD); // tell scheduler I have finished 
  return 0;
}

void CleanResources(int signum)
{
  exit(0);
}