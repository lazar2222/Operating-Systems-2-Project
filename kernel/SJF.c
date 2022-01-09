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

int SJFtype=SCHEDULER_SJF_EAGER_PREEMPRIVE;
int SJFtimeslice=1;
int SJFfactor=64;
int SJFease=1;

struct spinlock SJFsched_lock;

void initializeSJF()
{
    initlock(&SJFsched_lock,"SJFlock");
}

void perCoreInitializeSJF(int core)
{

}

int getSJF()
{
    acquire(&SJFsched_lock);
    if(heap[0]==0){release(&SJFsched_lock); return -1;}
    int index= heap[1];
    heapRemove(1);
    release(&SJFsched_lock);
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
        //printf("inserting with tau: %d %d %d\n",((proc[processIndex].schedtmp * (128 - SJFfactor)) + (proc[processIndex].executiontime * SJFfactor)+65)/128,proc[processIndex].schedtmp,proc[processIndex].executiontime);
        proc[processIndex].schedtmp = ((proc[processIndex].schedtmp * (128 - SJFfactor)) + (proc[processIndex].executiontime * SJFfactor)+65)/128;
        proc[processIndex].priority=proc[processIndex].schedtmp;
        proc[processIndex].executiontime=0;

    }
    else
    {
        //printf("inserting with live tau: %d %d %d\n",proc[processIndex].priority,proc[processIndex].schedtmp,proc[processIndex].executiontime);
    }
    proc[processIndex].state=RUNNABLE;
    acquire(&SJFsched_lock);
    heapInsert(processIndex);
    release(&SJFsched_lock);
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
        acquire(&SJFsched_lock);
        if(proc[heap[1]].priority<p->priority)
        {
            release(&SJFsched_lock);
            //printf("PREMPT %d\n",p->pid);
            yield();
        }
        else
        {
            release(&SJFsched_lock);
        }
    }
}

struct schedulingStrategy SJFscheduler={initializeSJF,perCoreInitializeSJF,getSJF, putSJF,timerSJF};