#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"

struct schedulingStrategy currentSchedulingStrategy;

struct spinlock sched_lock;

void sched_initialize()
{
    initlock(&sched_lock,"sched_lock");
    currentSchedulingStrategy.initialize();
}

void sched_perCoreInitialize(int core)
{
    currentSchedulingStrategy.perCoreInitialize(core);
}

int sched_get()
{
    int val;
    acquire(&sched_lock);
    val=currentSchedulingStrategy.get();
    release(&sched_lock);
    return val;
}

void sched_put(int processIndex,int reason)
{
    acquire(&sched_lock);
    currentSchedulingStrategy.put(processIndex,reason);
    release(&sched_lock);
}

void sched_timer(int user)
{
    currentSchedulingStrategy.timer(user);
}