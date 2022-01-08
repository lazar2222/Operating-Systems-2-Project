#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "CFS.h"

void initializeCFS()
{
    panic("CFS");
}

void perCoreInitializeCFS(int core)
{

}

int getCFS()
{
    return 0;
}

void putCFS(int processIndex,int reason)
{

}

void timerCFS(int user)
{

}

struct schedulingStrategy CFSscheduler={initializeCFS,perCoreInitializeCFS,getCFS, putCFS,timerCFS};