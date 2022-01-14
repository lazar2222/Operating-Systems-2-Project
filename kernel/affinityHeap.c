#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"
#include "affinityHeap.h"

int AHinitialLaziness=2;
int AHmaxAge=2;

int heaps[NCPU+2][NPROC+1]={{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
//HEAP 0 COVERING HEAP
//HEAP 1 NO AFFINITY HEAP
//HEAP 2 CPU 0 HEAP
//..
//HEAP X[0] SIZE
int backmap[NCPU+2][NPROC]={{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};

int roundCounter[NCPU]={0,0,0,0,0,0,0,0};
int agingHeads[NCPU][NPROC];
int agingPointer[NCPU]={0,0,0,0,0,0,0,0};
int totalSize=0;

int checkBackmap()
{
    for (int i = 0; i < NCPU+2; ++i) {
        if(heaps[i][0]<0)
        {
            return 0;
        }
        for (int j = 0; j < NPROC; ++j) {
            if(backmap[i][j]!=-1 && heaps[i][backmap[i][j]]!=j)
            {
                return 0;
            }
        }
        for (int j = 1; j <= heaps[i][0] ; ++j) {
            if(backmap[i][heaps[i][j]]!=j || heaps[i][j]==-1)
            {
                return 0;
            }
        }
    }
    return 1;
}

int AHheapUpdate(int heapIndex,int procIndex);

void AHinit()
{
    for (int i = 0; i < NCPU+2; ++i) {
        heaps[i][0]=0;
        for (int j = 0; j < NPROC; ++j) {
            backmap[i][j]=-1;
            heaps[i][j+1]=-1;
        }
    }
    for (int i = 0; i < NCPU; ++i) {
        for (int j = 0; j < NPROC; ++j) {
            agingHeads[i][j]=-1;
        }
    }
    for (int i = 0; i < NCPU; ++i) {
        agingPointer[i]=0;
        roundCounter[i]=0;
    }
    totalSize=0;
}

int AHget(int core)
{
    if(heaps[0][0]==0) // All heaps empty
    {
        //printf("EMPTY\n");
        return -1;
    }
    int ret;
    if(proc[heaps[0][1]].affinity==core || proc[heaps[0][1]].affinity==-1 || proc[heaps[0][1]].laziness==0)
    {
        ret=heaps[0][1];
        if(proc[heaps[0][1]].laziness==0)
        {
            printf("LAZY %d\n",ret);
        }
        else
        {
            printf("MIN %d\n",ret);
        }
    }
    else if(heaps[1][0]==0)
    {
        if(heaps[core+2][0]==0)
        {
            if(totalSize+runningProc<getcpunum())
            {
                return -1;
                printf("NOSTEAL\n");
            }
            ret=heaps[0][1];
            printf("STEAL %d\n",ret);
        }
        else
        {
            ret=heaps[core+2][1];
            printf("CORE %d\n",ret);
        }
    }
    else
    {
        if(heaps[core+2][0]==0)
        {
            ret=heaps[1][1];
            printf("NAF %d\n",ret);
        }
        else
        {
            if(proc[heaps[core+2][1]].priority<=proc[heaps[1][1]].priority)
            {
                ret=heaps[core+2][1];
                printf("CORECORE %d\n",ret);
            }
            else
            {
                ret=heaps[1][1];
                printf("NAFNAF %d\n",ret);
            }
        }
    }
    proc[heaps[0][1]].laziness--;
    //UPDATING

    int heapIndex=proc[ret].affinity+2;
    checkBackmap();
    heaps[heapIndex][1]=heaps[heapIndex][heaps[heapIndex][0]];
                                                                                                                                heaps[heapIndex][heaps[heapIndex][0]]=-1;
    backmap[heapIndex][ret]=-1;
    if(heaps[heapIndex][0]!=1)
    {
        backmap[heapIndex][heaps[heapIndex][1]] = 1;
    }
    heaps[heapIndex][0]--;
    checkBackmap();
    AHheapUpdate(heapIndex,1);

    checkBackmap();

    int mainIndex=backmap[0][ret];
    if(heaps[heapIndex][0]==0)
    {
        heaps[0][mainIndex]=heaps[0][heaps[0][0]];
                                                                                                                                heaps[0][heaps[0][0]]=-1;
        heaps[0][0]--;
    }
    else
    {
        heaps[0][mainIndex]=heaps[heapIndex][1];
    }
    backmap[0][ret]=-1;
    if(heaps[0][0]!=0)
    {
        backmap[0][heaps[0][mainIndex]] = mainIndex;
    }
    checkBackmap();
    if(mainIndex<=heaps[0][0])
    {
        AHheapUpdate(0, mainIndex);
    }

    checkBackmap();

    totalSize--;
    roundCounter[core]++;

    if(affinityEN==AFFINITY_ENABLED_AGING)
    {
        if (agingHeads[core][agingPointer[core]] != -1 && proc[agingHeads[core][agingPointer[core]]].affinityAge == roundCounter[core] - AHmaxAge)
        {
            heapIndex = core + 2;
            int procToExpunge = agingHeads[core][agingPointer[core]];
            if (backmap[heapIndex][procToExpunge] != -1)
            {
                printf("AGE %d\n",procToExpunge);
                checkBackmap();
                int posInHeap = backmap[heapIndex][procToExpunge];
                checkBackmap();
                if (posInHeap == 1)
                {
                    proc[procToExpunge].laziness = 0;
                }
                else
                {
                    checkBackmap();
                    heaps[heapIndex][posInHeap] = heaps[heapIndex][heaps[heapIndex][0]];
                                                                                                                            heaps[heapIndex][heaps[heapIndex][0]]=-1;
                    backmap[heapIndex][procToExpunge] = -1;
                    if(heaps[heapIndex][0]!=posInHeap)
                    {
                        backmap[heapIndex][heaps[heapIndex][posInHeap]] = posInHeap;
                    }
                    heaps[heapIndex][0]--;
                    checkBackmap();
                    AHheapUpdate(heapIndex, 1);

                    checkBackmap();

                    proc[procToExpunge].affinity = -1;

                    heaps[1][++heaps[1][0]] = procToExpunge;
                    backmap[1][procToExpunge] = heaps[1][0];
                    checkBackmap();
                    int lproc = heaps[1][1];
                    if (AHheapUpdate(1, heaps[1][0]) == 1)
                    {
                        checkBackmap();
                        if (heaps[1][0] == 1)
                        {
                            checkBackmap();
                            heaps[0][++heaps[0][0]] = procToExpunge;
                            backmap[0][procToExpunge] = heaps[0][0];
                            checkBackmap();
                            AHheapUpdate(0, heaps[0][0]);
                        }
                        else
                        {
                            checkBackmap();
                            heaps[0][backmap[0][lproc]] = procToExpunge;
                            backmap[0][procToExpunge] = backmap[0][lproc];
                            backmap[0][lproc] = -1;
                            checkBackmap();
                            AHheapUpdate(0, backmap[0][procToExpunge]);
                        }
                        checkBackmap();
                    }

                    checkBackmap();
                }
            }
        }
    }

    //Setting parameters
    proc[ret].affinity=core;
    proc[ret].affinityAge=roundCounter[core];
    agingHeads[core][agingPointer[core]]=ret;
    agingPointer[core]=(agingPointer[core]+1)%AHmaxAge;
    return ret;
}

void AHput(int procIndex)
{
    totalSize++;
    int heapIndex=proc[procIndex].affinity+2;
    proc[procIndex].laziness=AHinitialLaziness;
    //printf("PUT\n");
    checkBackmap();
    heaps[heapIndex][++heaps[heapIndex][0]]=procIndex;
    backmap[heapIndex][procIndex]=heaps[heapIndex][0];
    checkBackmap();
    int lproc=heaps[heapIndex][1];
    if(AHheapUpdate(heapIndex,heaps[heapIndex][0])==1)
    {
        checkBackmap();
        //printf("TUP\n");
        if(heaps[heapIndex][0]==1)
        {
            checkBackmap();
            heaps[0][++heaps[0][0]]=procIndex;
            backmap[0][procIndex]=heaps[0][0];
            checkBackmap();
            AHheapUpdate(0,heaps[0][0]);
            checkBackmap();
        }
        else
        {
            checkBackmap();
            heaps[0][backmap[0][lproc]]=procIndex;
            backmap[0][procIndex]=backmap[0][lproc];
            backmap[0][lproc]=-1;
            checkBackmap();
            AHheapUpdate(0,backmap[0][procIndex]);
            checkBackmap();
        }
        checkBackmap();
    }
    checkBackmap();
}

void AHclear()
{
    for (int i = 0; i < NCPU+2; ++i) {
        heaps[i][0]=0;
        for (int j = 0; j < NPROC; ++j) {
            backmap[i][j]=-1;
            heaps[i][j+1]=-1;
        }
    }
}

int AHmin(int core)
{
    if(heaps[0][0]==0) // All heaps empty
    {
        return -1;
    }
    if(proc[heaps[0][1]].affinity==core || proc[heaps[0][1]].affinity==-1 || proc[heaps[0][1]].laziness==0)
    {
        return heaps[0][1];
    }
    if(heaps[1][0]==0)
    {
        if(heaps[core+2][0]==0)
        {
            if(totalSize+runningProc<getcpunum())
            {
                return -1;
            }
            return heaps[0][1];
        }
        else
        {
            return heaps[core+2][1];
        }
    }
    else
    {
        if(heaps[core+2][0]==0)
        {
            return heaps[1][1];
        }
        else
        {
            if(proc[heaps[core+2][1]].priority<=proc[heaps[1][1]].priority)
            {
                return heaps[core+2][1];
            }
            else
            {
                return heaps[1][1];
            }
        }
    }
}

int AHheapUpdate(int heapIndex,int procIndex)
{
    //if(heaps[heapIndex][0]==0){ panic("removal from empty heap");}
    //if(procIndex < 1 || procIndex > heaps[heapIndex][0]){ panic("invalid proc index");}
    //heaps[heapIndex][procIndex]=heaps[heapIndex][heaps[heapIndex][0]];
    //heaps[heapIndex][0]--;
    int tmp;
    while(procIndex > 1)
    {
        if(proc[heaps[heapIndex][procIndex]].priority < proc[heaps[heapIndex][procIndex / 2]].priority)
        {
            checkBackmap();
            tmp=heaps[heapIndex][procIndex];
            heaps[heapIndex][procIndex]=heaps[heapIndex][procIndex / 2];
            heaps[heapIndex][procIndex / 2]=tmp;
            backmap[heapIndex][heaps[heapIndex][procIndex]]=procIndex;
            backmap[heapIndex][heaps[heapIndex][procIndex/2]]=procIndex/2;
            procIndex= procIndex / 2;
            checkBackmap();
        }
        else
        {
            break;
        }
    }
    while(2 * procIndex <= heaps[heapIndex][0])
    {
        if(proc[heaps[heapIndex][procIndex]].priority > proc[heaps[heapIndex][2 * procIndex]].priority)
        {
            if(proc[heaps[heapIndex][2 * procIndex]].priority > proc[heaps[heapIndex][(2 * procIndex) + 1]].priority && (2 * procIndex + 1) <= heaps[heapIndex][0])
            {
                //swap right
                checkBackmap();
                tmp=heaps[heapIndex][procIndex];
                heaps[heapIndex][procIndex]=heaps[heapIndex][(2 * procIndex) + 1];
                heaps[heapIndex][(2 * procIndex) + 1]=tmp;
                backmap[heapIndex][heaps[heapIndex][procIndex]]=procIndex;
                backmap[heapIndex][heaps[heapIndex][(2 * procIndex) + 1]]=(2 * procIndex) + 1;
                procIndex= (2 * procIndex) + 1;
                checkBackmap();
            }
            else
            {
                //swap left
                checkBackmap();
                tmp=heaps[heapIndex][procIndex];
                heaps[heapIndex][procIndex]=heaps[heapIndex][2 * procIndex];
                heaps[heapIndex][2 * procIndex]=tmp;
                backmap[heapIndex][heaps[heapIndex][procIndex]]=procIndex;
                backmap[heapIndex][heaps[heapIndex][2 * procIndex]]=2 * procIndex;
                procIndex=(2 * procIndex);
                checkBackmap();
            }
        }
        else if(proc[heaps[heapIndex][procIndex]].priority > proc[heaps[heapIndex][(2 * procIndex) + 1]].priority && (2 * procIndex + 1) <= heaps[heapIndex][0])
        {
            //swap right
            checkBackmap();
            tmp=heaps[heapIndex][procIndex];
            heaps[heapIndex][procIndex]=heaps[heapIndex][(2 * procIndex) + 1];
            heaps[heapIndex][(2 * procIndex) + 1]=tmp;
            backmap[heapIndex][heaps[heapIndex][procIndex]]=procIndex;
            backmap[heapIndex][heaps[heapIndex][(2 * procIndex) + 1]]=(2 * procIndex) + 1;
            procIndex= (2 * procIndex) + 1;
            checkBackmap();
        }
        else
        {
            break;
        }
    }
    return procIndex;
}