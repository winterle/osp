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

/* thread control block */
typedef struct tcb_s
{
	int tid;
	/* data needed to restore the context */
	ucontext_t context;
	char stackMem[STACK_SIZE];
} tcb_t;

void ult_init(ult_f f)
{
	tcb_t initThread;
	getcontext(&initThread.context); //initialize the initThread context with ours, might not be necessary (initializes all variables though)

	/*create an empty stack for initThread*/
	initThread.context.uc_link = 0;
	initThread.context.uc_stack.ss_flags = 0; // no blocked interrupts during execution of this thread
	initThread.context.uc_stack.ss_size = STACK_SIZE;
	initThread.context.uc_stack.ss_sp = initThread.stackMem; //set stackPointer of initThread to stackMem (Question: does stack not build up to lower indices?)
	initThread.tid = 0; //initThread has tid 0, should be the first in our context/thread managing array
	if(initThread.context.uc_stack.ss_sp  == NULL){} //todo catch error

	/* set the instructionPointer to the provided function by modifying the context */
	makecontext(&initThread.context, f, 0);
    /* todo save the initThread in threadArray, [alloc dynamically] <- not necessary*/

	/* todo schedule this thread in the scheduleQueue and return
	 * todo or should we execute this thread now? (eg. load the context)  if so, we might have to save the currContext somewhere to return here after
	 * todo the provided function returns (might just not return, if specified so, it's fine) */
    setcontext(&initThread.context);
}

int ult_spawn(ult_f f)
{	
	return 0;		
}

void ult_yield()
{}

void ult_exit(int status)
{}

int ult_join(int tid, int* status)
{
	return -1;
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
	return 0;
}
