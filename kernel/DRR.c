#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"

struct proc *p;

void initializeDRR()
{
    p=proc;
}

void perCoreInitializeDRR(int core)
{

}

int getDRR(int core)
{
    for(; p < &proc[NPROC]; p++) {

        if(p->state == RUNNABLE) {
            acquire(&p->lock);
            p->state = RUNNING;
            //printf("getDRR\n");
            return (p++)-proc;
        }
    }
    p=proc;
    return -1;
}

void putDRR(int processIndex,int reason)
{
    //printf("putDRR\n");
    proc[processIndex].state = RUNNABLE;
}

void timerDRR(int user)
{
    //printf("timerDRR\n");
    yield();
}

struct schedulingStrategy DRRscheduler={initializeDRR,perCoreInitializeDRR,getDRR, putDRR,timerDRR};
