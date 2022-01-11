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

struct spinlock CFSsched_lock;

void initializeCFS()
{
    initlock(&CFSsched_lock,"CFSlock");
}

void perCoreInitializeCFS(int core)
{

}

int getCFS()
{
    int wticks;
    acquire(&CFSsched_lock);
    if(heap[0]==0){release(&CFSsched_lock); return -1;}
    int index= heap[1];
    heapRemove(1);
    release(&CFSsched_lock);
    wticks=ticks;
    proc[index].timeslice=(wticks-proc[index].schedtmp+(getprocnum()/2))/getprocnum();
    if(proc[index].timeslice==0){proc[index].timeslice=1;}
    //printf("TS %d %d %d\n",wticks-proc[index].schedtmp,getprocnum(),proc[index].timeslice);
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
    //printf("PUT %d %d\n",proc[processIndex].priority,proc[processIndex].schedtmp);
    proc[processIndex].state=RUNNABLE;
    acquire(&CFSsched_lock);
    heapInsert(processIndex);
    release(&CFSsched_lock);
}

void timerCFS(int user)
{
    struct proc* p=myproc();
    p->executiontime++;
    //printf("TIMER\n");
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