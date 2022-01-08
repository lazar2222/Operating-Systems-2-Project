#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "SJF.h"

void initializeSJF()
{
    panic("SJF");
}

void perCoreInitializeSJF(int core)
{

}

int getSJF()
{
    return 0;
}

void putSJF(int processIndex,int reason)
{

}

void timerSJF(int user)
{

}

struct schedulingStrategy SJFscheduler={initializeSJF,perCoreInitializeSJF,getSJF, putSJF,timerSJF};