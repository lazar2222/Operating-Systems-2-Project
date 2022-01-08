#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "heap.h"

int heap[NPROC+1]={0};

void heapInsert(int procIndex)
{
    if(heap[0]==NPROC){ panic("scheduler heap full");}
    heap[++heap[0]]=procIndex;
    int i=heap[0];
    int tmp;
    while(i>1)
    {
        if(proc[heap[i]].priority<proc[heap[i/2]].priority)
        {
            tmp=heap[i];
            heap[i]=heap[i/2];
            heap[i/2]=tmp;
            i=i/2;
        }
        else
        {
            break;
        }
    }
}

int min(int a,int b)
{
    return  a<b?a:b;
}

void heapRemove(int heapIndex)
{
    if(heap[0]==0){ panic("removal from empty heap");}
    if(heapIndex<1 || heapIndex>heap[0]){ panic("invalid heap index");}
    int tmp;
    heap[heapIndex]=heap[heap[0]];
    while(heapIndex>1)
    {
        if(proc[heap[heapIndex]].priority<proc[heap[heapIndex/2]].priority)
        {
            tmp=heap[heapIndex];
            heap[heapIndex]=heap[heapIndex/2];
            heap[heapIndex/2]=tmp;
            heapIndex=heapIndex/2;
        }
        else
        {
            break;
        }
    }
    while(2*heapIndex<=heap[0])
    {
        if(proc[heap[heapIndex]].priority>proc[heap[2*heapIndex]].priority)
        {
            if(proc[heap[2*heapIndex]].priority>proc[heap[(2*heapIndex)+1]].priority && (2*heapIndex+1)<=heap[0])
            {
                //swap right
                tmp=heap[heapIndex];
                heap[heapIndex]=heap[(2*heapIndex)+1];
                heap[(2*heapIndex)+1]=tmp;
                heapIndex=(2*heapIndex)+1;
            }
            else
            {
                //swap left
                tmp=heap[heapIndex];
                heap[heapIndex]=heap[2*heapIndex];
                heap[2*heapIndex]=tmp;
                heapIndex=(2*heapIndex);
            }
        }
        else if(proc[heap[heapIndex]].priority>proc[heap[(2*heapIndex)+1]].priority && (2*heapIndex+1)<=heap[0])
        {
            //swap right
            tmp=heap[heapIndex];
            heap[heapIndex]=heap[(2*heapIndex)+1];
            heap[(2*heapIndex)+1]=tmp;
            heapIndex=(2*heapIndex)+1;
        }
        else
        {
            break;
        }
    }
    heap[0]--;
}

void heapify()
{
    //TODO: LOCK
    int size=heap[0];
    if(size<2){return;}
    heap[0]=1;
    for (int i = 2; i <= size; ++i) {
        heapInsert(heap[i]);
    }
}