#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/scheduler.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: sched type|affinity [args]\n");
      printf("Default Round Robin Scheduler: sched DRR\n");
      printf("Shortest Job First Scheduler: sched SJF factor\n");
      printf("Preemptive Shortest Job First Scheduler: sched SJF_PRE factor timeslice\n");
      printf("Eager Preemptive Shortest Job First Scheduler: sched SJF_EAGER_PRE factor timeslice tolerance\n");
      printf("Completely Fair Scheduler: sched CFS\n");
      printf("Setting affinity parameters: sched AFFINITY_ENABLED_AGING|AFFINITY_ENABLED|AFFINITY_DISABLED initialLaziness maxAge\n");
      printf("Parameters are positional, value of -1 indicates no change\n");
    exit(1);
  }

  int values[3]={-1,-1,-1};
  int ret=1;

  for (int i=2;i<argc;i++)
  {
    int tmp=atoi(argv[i]);
    if(tmp!=NOT_AN_INT)
    {
        values[i-2]=tmp;
    }
    else
    {
        printf("sched: argument %d is not a number\n",i);
        exit(1);
    }
  }

  if(strcmp(argv[1],"DRR")==0)
  {
      if(argc>2)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = sched(SCHEDULER_DRR,values[0],values[1],values[2]);
  }
  else if(strcmp(argv[1],"SJF")==0)
  {
      if(argc>3)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = sched(SCHEDULER_SJF,values[0],values[1],values[2]);
  }
  else if(strcmp(argv[1],"SJF_PRE")==0)
  {
      if(argc>4)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = sched(SCHEDULER_SJF_PREEMPRIVE,values[0],values[1],values[2]);
  }
  else if(strcmp(argv[1],"SJF_EAGER_PRE")==0)
  {
      if(argc>5)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = sched(SCHEDULER_SJF_EAGER_PREEMPRIVE,values[0],values[1],values[2]);
  }
  else if(strcmp(argv[1],"CFS")==0)
  {
      if(argc>2)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = sched(SCHEDULER_CFS,values[0],values[1],values[2]);
  }
  else if(strcmp(argv[1],"AFFINITY_ENABLED_AGING")==0)
  {
      if(argc>4)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = affinity(AFFINITY_ENABLED_AGING,values[0],values[1]);
  }
  else if(strcmp(argv[1],"AFFINITY_ENABLED")==0)
  {
      if(argc>4)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = affinity(AFFINITY_ENABLED,values[0],values[1]);
  }
  else if(strcmp(argv[1],"AFFINITY_DISABLED")==0)
  {
      if(argc>4)
      {
          printf("sched: too many arguments\n");
          exit(1);
      }
      ret = affinity(AFFINITY_DISABLED,values[0],values[1]);
  }
  else
  {
      printf("sched: invalid first argument\n");
      exit(1);
  }

    printf(ret==0?"sched: SUCCESS %d\n":"sched: FAILED %d\n",ret);

  exit(0);
}
