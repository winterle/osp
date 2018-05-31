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
#define STACK_SIZE (1024*1024)
#define MAX_THREADS 1024

/* fixme Problems:
 * there are a few invalid reads at setcontext, no idea why (might have to look at the source code), can see when running valgrind
 * have to free the runQueueTid's, memory leak right now
 * 
 * bunch of other bugs/not yet implemented things (look at code)
 * */

typedef struct linkedListNode{
    int tid;
    int exitCode;
    int waitingFor;
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
static runQueueTid * rq_GetLast(runQueueTid *from);
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
    runQueueTid *last = rq_GetLast(root);
    if(last != NULL){
        rq_GetLast(root)->next = new;
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

void ult_exit(int status)
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
            printf("Thread %d finished with exit code %d\n",activeThreadTid,this->exitCode);
            roundRobin();
        }
    }
}

int ult_join(int tid, int* status)
{
    //todo free those reaped runQueueTids
    rq_GetTid(activeThreadTid)->waitingFor = tid;
    runQueueTid *finished = rq_GetTid(tid);
    if(finished == NULL)return -1; //this thread never existed or was already reaped
    if(finished->exitCode != INT_MAX) {
        printf("Has already finished, returning exit code \n");
        *status = finished->exitCode;
        return 0;
    }

    printf("Tid has not finished yet, capturing context..\n");
    /* the thread has not finished yet, so we capture the context and let the scheduler take over */
    ucontext_t saveContext;
    getcontext(&saveContext);
    if(ret == 1){//todo we just restored the previously saved context, return to the thread with correct return code
        ret = 0;
        printf("Context sucessfully restored \n");
        if(rq_GetTid(activeThreadTid) == NULL)printf("something went wrong, rq_gettid of active thread tid returned NULL");
        printf("Waited for tid %d\n",rq_GetTid(activeThreadTid)->waitingFor);
        int exitCode = rq_GetTid(rq_GetTid(activeThreadTid)->waitingFor)->exitCode;
        printf("Exit code was %d\n",exitCode);
        *status = exitCode;
        if(exitCode != INT_MAX) {
            *status = exitCode;
            return 0;
        }
        else return -1; //todo the requested thread has not finished yet, call the scheduler again (no need to save context, last one is still ok)

    }
    printf("Saving the context\n");
    (threadArray)[activeThreadTid].context = saveContext;
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
    new->waitingFor = INT_MAX;
    return new;
}

static runQueueTid * rq_GetLast(runQueueTid *from){
    if(from == NULL){
        return NULL;
    }
    runQueueTid * curr = from;
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
    rq_GetLast(root)->next = root;
    root->next = NULL;
    if(newRoot == NULL){exit(-1);}//todo there is only one thread, return
    while(newRoot->exitCode != INT_MAX){
        rq_GetLast(newRoot)->next = newRoot;
        runQueueTid *this = newRoot->next;
        newRoot->next = NULL;
        newRoot = this;
    }
    root = newRoot;
    activeThreadTid = root->tid;
    printf("New activeThreadTid = %d\n",activeThreadTid);
    /* array.h might realloc or move this element, but it does not matter because this pointer is only used once in this local context */
    ucontext_t *setTo = &(threadArray)[activeThreadTid].context;
    ret = 1;
    setcontext(setTo);
}