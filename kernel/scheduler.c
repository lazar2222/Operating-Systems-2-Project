#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "SJF.h"
#include "CFS.h"
#include "DRR.h"
#include "heap.h"
#include "affinityHeap.h"

struct schedulingStrategy* currentSchedulingStrategy;

struct spinlock sched_lock;

int runningProc=0;
int runningBitFlag=0;
int affinityEN;

void sched_initialize()
{
    currentSchedulingStrategy=&CFSscheduler;
    SJFtype=SCHEDULER_SJF_EAGER_PREEMPRIVE;
    SJFfactor=64;
    SJFtimeslice=1;
    SJFease=1;
    affinityEN=AFFINITY_ENABLED_AGING;
    AHinitialLaziness=2;
    AHmaxAge=2;
    initlock(&sched_lock,"sched_lock");
    AHinit();
    currentSchedulingStrategy->initialize();
}

void sched_perCoreInitialize(int core)
{
    currentSchedulingStrategy->perCoreInitialize(core);
}

int sched_get(int core)
{
    int val;
    acquire(&sched_lock);
    val=currentSchedulingStrategy->get(core);
    int nrp=runningBitFlag;
    if(val==-1)
    {
        nrp&=~(1<<core);
    }
    else
    {
        nrp|=(1<<core);
    }
    if(nrp!=runningBitFlag)
    {
        if((nrp&(1<<core))!=0)
        {
            runningProc++;
            //printf("RUNNING: %d\n",runningProc);
        }
        else
        {
            runningProc--;
            //printf("RUNNING: %d\n",runningProc);
        }
    }
    runningBitFlag=nrp;
    release(&sched_lock);
    return val;
}

void sched_put(int processIndex,int reason)
{
    acquire(&sched_lock);
    currentSchedulingStrategy->put(processIndex,reason);
    release(&sched_lock);
}

void sched_timer(int user)
{
    currentSchedulingStrategy->timer(user);
}

int sched_change(int type,int factor,int timeslice,int ease)
{
    //Checking which scheduler are we switching to
    struct schedulingStrategy* newSS;
    if(type==SCHEDULER_DRR){newSS=&DRRscheduler;}
    else if(type==SCHEDULER_SJF || type==SCHEDULER_SJF_PREEMPRIVE || type==SCHEDULER_SJF_EAGER_PREEMPRIVE){newSS=&SJFscheduler;}
    else if(type==SCHEDULER_CFS){newSS=&CFSscheduler;}
    else{return -1;} //Unknown scheduler
    //Checking parameters
    if(factor!=-1 && (factor<0 || factor>128)){return -2;} //Invalid parameter
    if(timeslice!=-1 && timeslice<1){return -2;} //Invalid parameter
    if(ease!=-1 && ease<0){return -2;} //Invalid parameter
    //Changing schedulers,
    acquire(&sched_lock);   //Prevent scheduling while changing schedulers
                            //and since acquire disables interrupts it also prevents timer from being fired

    if(SJFtype!=type) {
        //Respect preemption and force rescheduling on preemptive schedulers
        //No need to update any other parameters if we are not changing scheduling algorithms
        int nts = type == SCHEDULER_SJF ? 0 : 1;
        for (int i = 0; i < NCPU; ++i) {
            if (cpus[i].proc != 0) {
                cpus[i].proc->timeslice = nts;
            }
        }
    }

    //We can always set parameters
    SJFtype=type;
    if(factor!=-1){SJFfactor=factor;}
    if(timeslice!=-1){SJFtimeslice=timeslice;}
    if(ease!=-1){SJFease=ease;}

    //Are we changing the scheduling algorithm
    if(currentSchedulingStrategy != newSS)
    {
        //We have to do housekeeping of heap and scheduling data in proc

        //Lets zero all scheduling data while we are at it
        for (int i = 0; i < NPROC; ++i) {
            proc[i].priority=0;
            proc[i].executiontime=0;
            proc[i].schedtmp=0;
        }

        if(currentSchedulingStrategy==&DRRscheduler)
        {
            for (int i = 0; i < NPROC; ++i) {
                if(proc[i].state==RUNNABLE)
                {
                    if(affinityEN!=AFFINITY_DISABLED)
                    {
                        AHput(i);
                    }
                    else
                    {
                        heapInsert(i);
                    }
                }
            }
        }
        if(newSS==&DRRscheduler)
        {
            if(affinityEN!=AFFINITY_DISABLED)
            {
                AHclear();
            }
            else
            {
                heapClear();
            }
            newSS->initialize();
        }

        //Actually changing schedulers
        currentSchedulingStrategy=newSS;
    }

    release(&sched_lock);

    return 0;
}

int sched_affinity(int enabled,int initialLaziness, int maxAge)
{
    if(enabled!=AFFINITY_DISABLED && enabled!=AFFINITY_ENABLED && enabled!=AFFINITY_ENABLED_AGING)
    {
        return -1; //Unknown affinity strategy
    }
    if(initialLaziness !=-1 && initialLaziness<0){return -2;}
    if(maxAge!=-1 && (maxAge<1 || maxAge>=NPROC)){return -2;}

    //I cant be bothered to check for cases so lets always do a full reset
    acquire(&sched_lock);   //Prevent scheduling while changing schedulers
                            //and since acquire disables interrupts it also prevents timer from being fired

    //Reset internal data structures
    AHinit();
    for (int i = 0; i < NPROC; ++i) {
        proc[i].affinity=-1;
        proc[i].affinityAge=0;
        proc[i].laziness=0;
    }

    //Migrate heaps
    if(affinityEN!=enabled)
    {
        if(affinityEN==AFFINITY_DISABLED)
        {
            heapClear();
            for (int i = 0; i < NPROC; ++i) {
                if(proc[i].state==RUNNABLE)
                {
                    AHput(i);
                }
            }
        }
        if(enabled==AFFINITY_DISABLED)
        {
            AHclear();
            for (int i = 0; i < NPROC; ++i) {
                if(proc[i].state==RUNNABLE)
                {
                    heapInsert(i);
                }
            }
        }
    }

    //Update parameters
    affinityEN=enabled;
    if(initialLaziness!=-1){AHinitialLaziness=initialLaziness;}
    if(maxAge!=-1){AHmaxAge=maxAge;}

    release(&sched_lock);
    return 0;
}