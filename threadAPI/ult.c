#include "ult.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#define _XOPEN_SOURCE
#include <ucontext.h>
#define STACK_SIZE (64*1024)
#define MAX_THREADS 1024

typedef struct linkedListNode{
    int tid;
    int exitCode;
    struct linkedListNode *next;
}runQueueTid;

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
runQueueTid *root;
short ret; //boolean, if we just loaded the context so we can decide, weather to return from yield() resp. join() or to continue working inside of these functions


static runQueueTid * rq_New();
static runQueueTid * rq_GetLast();
static runQueueTid * rq_GetTid(int tid);
static void roundRobin();


void ult_init(ult_f f)
{
    root = NULL; /* important: has to be at the very beginning, otherwise possible segmentation violations! */
    ret = 0;
    threadArray = (arrayInit)(8,sizeof(tcb_t)); //initializing a new Array using array.h
    if(threadArray == NULL){}//todo catch error

    if(ult_spawn(f)){};//todo catch error: should be zero, since it's the very first thread

    /* insert into runQueue */
    root = rq_New();
    root->next = NULL;
    root->tid = 0;
	/*
	 * todo or should we execute this thread now? (eg. load the context)  if so, we might have to save the currContext somewhere to return here after
	 * todo the provided function returns (might just not return, if specified so, it's fine) */
    tcb_t setTo = arrayTop(threadArray);
    activeThreadTid = setTo.tid;
    setcontext(&setTo.context);

}

int ult_spawn(ult_f f)
{
    tcb_t newThread;
    getcontext(&newThread.context);

    /*create the new stack*/
    newThread.context.uc_link = 0;
    newThread.context.uc_stack.ss_flags = 0; // no blocked signals during execution of this thread
    newThread.context.uc_stack.ss_size = STACK_SIZE;
    newThread.context.uc_stack.ss_sp = newThread.stackMem; //set stackPointer of initThread to stackMem (Question: does stack not build up to lower indices?)
    newThread.tid = (int)arrayCount(threadArray);
    makecontext(&newThread.context,f,0);
    arrayPush(threadArray) = newThread;

    /*insert this thread into run-queue (modulize it?)*/
    runQueueTid *new = rq_New();
    runQueueTid *last = rq_GetLast();
    if(last != NULL){
        rq_GetLast()->next = new;
        new->tid = newThread.tid;
    }
    else{
        free(new);
    }

	return newThread.tid;
}

void ult_yield()
{
    ucontext_t saveContext;
    getcontext(&saveContext);
    if(ret == 1){ret = 0;return;} //todo we just restored the previously saved context, return to the thread with correct return code
    (threadArray+activeThreadTid)[0].context = saveContext;
    roundRobin();
}

void ult_exit(int status)//only implemented for initialThread
{
    if(activeThreadTid == 0){//todo: check if all other threads are already reaped (not really necessary)
        arrayRelease(threadArray);
        exit(status); //fixme: how to return to the ip where init was called? (better: return to ult_init after the setcontext call) -> some global bool's
    }
    else{
        runQueueTid *this = rq_GetTid(activeThreadTid);
        if(this == NULL){exit(-1);}//todo catch error
        else{
            this->exitCode = status;
            (threadArray+this->tid)[0].tid = -1;
            this->tid = -1;
            //todo goto scheduler
        }
    }
}

int ult_join(int tid, int* status)
{
    //todo check if tid already finished, and return
    /* the thread has not finished yet, so we capture the context and let the scheduler take over */
    ucontext_t saveContext;
    getcontext(&saveContext);
    if(ret == 1){ret = 0;return 0;} //todo we just restored the previously saved context, return to the thread with correct return code
    (threadArray+activeThreadTid)[0].context = saveContext;
    roundRobin();
	return -1;
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
	return 0;
}

static runQueueTid * rq_New(){
    runQueueTid *new = malloc(sizeof(runQueueTid));
    new->next = NULL;
    new->tid = -1;
    new->exitCode = INT_MAX;
    return new;
}

static runQueueTid * rq_GetLast(){
    if(root == NULL){
        return NULL;
    }
    runQueueTid * curr = root;
    while(curr->next != NULL) curr = curr->next;
    return curr;
}

static runQueueTid * rq_GetTid(int tid){
    runQueueTid *curr = root;
    if(curr == NULL){
        return NULL;
    }
    while(curr->tid != tid && curr->next != NULL)curr = curr->next;
    if(curr->tid != tid)return NULL;
    return curr;
}

static void roundRobin(){
    runQueueTid *newRoot = root->next;
    if(newRoot == NULL){}//todo there is only one thread, return
    rq_GetLast()->next = root;
    while(newRoot->tid == -1)newRoot = newRoot->next; //todo we have to put them to the beginning of the queue (they are already finished), right now they are lost in the void
    root = newRoot;
    activeThreadTid = root->tid;
    tcb_t setTo = (threadArray+activeThreadTid)[0];
    ret = 1;
    setcontext(&setTo.context);
}