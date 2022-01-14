#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "CFS.h"
#include "heap.h"
#include "affinityHeap.h"

void initializeCFS()
{

}

void perCoreInitializeCFS(int core)
{

}

int getCFS(int core)
{
    int wticks;
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
    wticks=ticks;
    proc[index].timeslice=(wticks-proc[index].schedtmp+(getprocnum()/2))/getprocnum();
    if(proc[index].timeslice==0){proc[index].timeslice=1;}
    acquire(&(proc[index].lock));
    proc[index].state = RUNNING;
    return index;
}

void putCFS(int processIndex,int reason)
{
    if(reason==REASON_AWAKENED)
    {
        proc[processIndex].executiontime=0;
    }
    proc[processIndex].priority=proc[processIndex].executiontime;
    proc[processIndex].schedtmp = ticks;
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

void timerCFS(int user)
{
    struct proc* p=myproc();
    p->executiontime++;
    if(p->timeslice>1)
    {
        p->timeslice--;
    }
    else if(p->timeslice==1)
    {
        yield();
    }
}

struct schedulingStrategy CFSscheduler={initializeCFS,perCoreInitializeCFS,getCFS, putCFS,timerCFS};