#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "SJF.h"
#include "heap.h"
#include "affinityHeap.h"

int SJFtype=SCHEDULER_SJF_EAGER_PREEMPRIVE;
int SJFfactor=64;
int SJFtimeslice=1;
int SJFease=1;

void initializeSJF()
{

}

void perCoreInitializeSJF(int core)
{

}

int getSJF(int core)
{
    int index;
    if(affinityEN!=AFFINITY_DISABLED)
    {
        index = AHget(core);
        if (index == -1) { return -1; }
    }
    else
    {
        index = heapMin();
        if (index == -1) { return -1; }
        heapRemove(1);
    }
    if(SJFtype==SCHEDULER_SJF)
    {
        proc[index].timeslice=0;
    }
    else
    {
        proc[index].timeslice=SJFtimeslice;
    }
    acquire(&(proc[index].lock));
    proc[index].state = RUNNING;
    return index;
}

void putSJF(int processIndex,int reason)
{
    if(reason==REASON_AWAKENED)
    {
        proc[processIndex].schedtmp = ((proc[processIndex].schedtmp * (128 - SJFfactor)) + (proc[processIndex].executiontime * SJFfactor)+65)/128;
        proc[processIndex].priority=proc[processIndex].schedtmp;
        proc[processIndex].executiontime=0;
    }
    proc[processIndex].state=RUNNABLE;
    if(affinityEN!=AFFINITY_DISABLED)
    {
        AHput(processIndex);
    }
    else
    {
        heapInsert(processIndex);
    }
}

void timerSJF(int user)
{
    struct proc* p=myproc();
    p->executiontime++;
    if(p->timeslice>1)
    {
        p->timeslice--;
    }
    else if(p->timeslice==1)
    {
        //recheck scheduling calculate comparison priority "live tau"
        p->priority=(SJFtype==SCHEDULER_SJF_EAGER_PREEMPRIVE)?(((p->schedtmp*(128-SJFfactor))+(p->executiontime*SJFfactor)+65)/128)-SJFease:p->priority;
        acquire(&sched_lock);
        int hmin;
        if(affinityEN!=AFFINITY_DISABLED)
        {
            hmin=AHmin(mycpu()-cpus);
        }
        else
        {
            hmin=heapMin();
        }
        if(hmin!=-1 && proc[hmin].priority<p->priority)
        {
            release(&sched_lock);
            yield();
        }
        else
        {
            release(&sched_lock);
        }
    }
}

struct schedulingStrategy SJFscheduler={initializeSJF,perCoreInitializeSJF,getSJF, putSJF,timerSJF};