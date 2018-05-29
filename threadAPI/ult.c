#include "ult.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define _XOPEN_SOURCE
#include <ucontext.h>
#define STACK_SIZE (64*1024)
#define MAX_THREADS 1024



/* thread control block */
typedef struct tcb_s
{
	int tid;
	/* data needed to restore the context */
	ucontext_t context;
	char stackMem[STACK_SIZE];
} tcb_t;

tcb_t *threadArray;
int activeThreadTid;

void ult_init(ult_f f)
{
    threadArray = (arrayInit)(8,sizeof(tcb_t)); //initializing a new Array using array.h
    if(threadArray == NULL){}//todo catch error

    if(ult_spawn(f)){};//todo catch error: should be zero, since it's the very first thread

	/* todo schedule this thread in the scheduleQueue and return
	 * todo or should we execute this thread now? (eg. load the context)  if so, we might have to save the currContext somewhere to return here after
	 * todo the provided function returns (might just not return, if specified so, it's fine) */
    tcb_t setTo = arrayTop(threadArray);
    activeThreadTid = setTo.tid;
    setcontext(&setTo.context); //fixme is this defined conforming to array.h?

}

int ult_spawn(ult_f f)
{
    tcb_t newThread;
    getcontext(&newThread.context);

    /*create the new stack*/
    newThread.context.uc_link = 0;
    newThread.context.uc_stack.ss_flags = 0; // no blocked interrupts during execution of this thread
    newThread.context.uc_stack.ss_size = STACK_SIZE;
    newThread.context.uc_stack.ss_sp = newThread.stackMem; //set stackPointer of initThread to stackMem (Question: does stack not build up to lower indices?)
    newThread.tid = (int)arrayCount(threadArray);
    makecontext(&newThread.context,f,0);
    arrayPush(threadArray) = newThread;
    /*todo schedule this thread*/
	return newThread.tid;
}

void ult_yield()
{}

void ult_exit(int status)//only implemented for initialThread
{
    if(activeThreadTid == 0){//todo: check if all other threads are already reaped (not really necessary)
        arrayRelease(threadArray);
        exit(status); //fixme: how to return to the ip where init was called? (better: return to ult_init after the setcontext call)
    }
}

int ult_join(int tid, int* status)
{
	return -1;
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
	return 0;
}
