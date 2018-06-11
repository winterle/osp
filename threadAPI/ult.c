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

int FLAG_1=0;
int FLAG_2=0;
int FLAG_3=0;
int FLAG_4=0;
int FLAG_5=0;

/* fixme Problems:
 * there are a few invalid reads at setcontext, no idea why (might have to look at the source code), can see when running valgrind
 * have to free the runQueueTids, memory leak right now (partially fixed)
 * reusing tids is not possible right now
 * yield() does not seem to work properly -> stack smashing!
 * bunch of other bugs/not yet implemented things (look at code)
 * */

/* thread control block */
typedef struct tcb_s
{
	int tid;
    int exitCode;
    int waitingFor;
    int blocked;
    fd_set set;
    int biggest_fd;
    struct tcb_s *nextInQ;
	/* data needed to restore the context */
	ucontext_t context;
	char stackMem[STACK_SIZE];
} tcb_t;

int activeThreadTid;
tcb_t *nextThread;
int threadCount;
int *st;


//static void roundRobin();
static void die (const char *errorMessage);
static tcb_t * getTCBbyTID(int tid);
static int rr_getNext();
static void print();


void ult_init(ult_f f)
{
    threadCount = 0;
    nextThread = NULL;
    int tid;
    if((tid = ult_spawn(f))){
        die("Spawning the initial thread failed\n");
    }

    /* insert into runQueue */
    activeThreadTid = tid;
    if(FLAG_4==1)printf("intial thread spawned\n");
    setcontext(&nextThread->context);

}

int ult_spawn(ult_f f)
{
    tcb_t *newThread = malloc(sizeof(tcb_t));
    getcontext(&newThread->context);

    newThread->exitCode = INT_MAX;
    newThread->waitingFor = INT_MAX;
    newThread->nextInQ = nextThread;
    newThread->blocked = 0;
    FD_ZERO(&(newThread->set));
    newThread->biggest_fd=-1;


    /*create the new stack*/
    newThread->context.uc_link = 0;
    newThread->context.uc_stack.ss_flags = 0; // no blocked signals during execution of this thread
    newThread->context.uc_stack.ss_size = STACK_SIZE;
    newThread->context.uc_stack.ss_sp = newThread->stackMem; //set stackPointer of initThread to stackMem
    newThread->tid = threadCount++;
    if(FLAG_4==1)printf("new thread spawned, tid = %d\n",newThread->tid);
    makecontext(&newThread->context,f,0);

    /*insert this thread into run-queue*/
    nextThread = newThread;


	return newThread->tid;
}

void ult_yield()
{
    int tid = activeThreadTid;
    int nextTid = rr_getNext();
    swapcontext(&getTCBbyTID(tid)->context,&getTCBbyTID(nextTid)->context);
}

void ult_exit(int status)
{
    if(FLAG_4==1)printf("tid %d finished\n",activeThreadTid);
    if(activeThreadTid == 0){//todo: check if all other threads are already reaped (not really necessary)
        tcb_t *curr = nextThread;
        while(curr->nextInQ!=NULL && curr!=NULL){
            tcb_t *this = curr;
            curr = curr->nextInQ;
            if(FLAG_1==1) printf("freeing %d",this->tid); //out
            free(this);
        }
        if(FLAG_1==1)printf("freeing %d",curr->tid); //out
        free(curr);
        exit(status); //fixme: how to return to the ip where init was called? (better: return to ult_init after the setcontext call) -> some global bool's
    }
    else{
        tcb_t *self = getTCBbyTID(activeThreadTid);
        if(self == NULL)return; //this thread does not exist
        else self->exitCode = status;
        int next = rr_getNext();
        swapcontext(&self->context,&getTCBbyTID(next)->context);
    }
}


int ult_join(int tid, int* status)
{
    st = status;
    if(FLAG_2==1)printf("join\n");
    tcb_t *self = getTCBbyTID(activeThreadTid);
    if(self == NULL){
        if(FLAG_4==1)printf("self is NULL\n");
        return -1;
    }
    tcb_t *waitingFor = getTCBbyTID(tid);
    if(waitingFor == NULL){
        if(FLAG_4==1)printf("waiting for is NULL");
        return -1;
    } //this thread never existed or was already reaped

    if(waitingFor->exitCode != INT_MAX) {
        if(FLAG_4==1)printf("Has already finished, returning exit code \n");
        *st = waitingFor->exitCode;
        return 0;
    }

    if(FLAG_4==1)printf("Tid has not finished yet, swapping contexts...\n");
    /* the thread has not finished yet, so we capture the context and let the scheduler take over */
    int nextTid = rr_getNext();
    swapcontext(&self->context,&getTCBbyTID(nextTid)->context);
    /*  */
    swap:    if(FLAG_4==1)printf("context restored\n");
    if(FLAG_4==1)printf("Waited for tid %d\n",tid);
    int exitCode = waitingFor->exitCode;
    if(exitCode != INT_MAX) {
        *st = exitCode;
        return 0;
    }
    else {
        if(FLAG_4==1)printf("waiting some more\n");
        nextTid = rr_getNext();
        swapcontext(&self->context,&getTCBbyTID(nextTid)->context);
        goto swap;
    }
}

int check(int tid){

    struct timeval timeout;
    int rv;
    tcb_t *curr = getTCBbyTID(tid);
    fd_set set=curr->set;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50;
    if(FLAG_3==1)printf("thread= %d : fd= %d\n",tid,curr->biggest_fd);

    if(curr->biggest_fd==-1){
        return 1;
    }

    rv = select(curr->biggest_fd + 1, &set, NULL, NULL, &timeout);
    if(rv == -1){
        fprintf(stderr,"select error (-1)");
        exit(-1);
    }else if(rv == 0) {
        if(FLAG_3==1) printf("tid=%d no data\n",tid);
        curr->blocked=1;
        return 0;
    }else{
        if(FLAG_5==1) printf("tid=%d data ist da\n",tid);
        curr->blocked=0;
        return 1;
    }
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
    int byte =0;
    if(FLAG_2==1)printf("fd=%d\n",fd);
    if(FLAG_2==1)printf("big_fd=%d\n",getTCBbyTID(activeThreadTid)->biggest_fd);
    if(fd>=(getTCBbyTID(activeThreadTid)->biggest_fd)){
        getTCBbyTID(activeThreadTid)->biggest_fd=fd;
    }


     /* clear the set */
    FD_SET(fd, &(getTCBbyTID(activeThreadTid)->set)); /* add our file descriptor to the set */

    if(check(activeThreadTid)==0){
        ult_yield();
    }

    byte = read(fd, buf, size); /* there was data to read */
        if(byte<0){
            fprintf(stderr,"ult_read : read error");
            exit(-1);
        }
        return byte;
}

static int rr_getNext(){
    int tid = nextThread->tid;
    tcb_t *last = nextThread;
    while(last->nextInQ!=NULL){
        last = last->nextInQ;
    }
    last->nextInQ=nextThread;
    last = nextThread;
    nextThread = nextThread->nextInQ;
    last->nextInQ = NULL;
    activeThreadTid = tid;
    if(FLAG_3==1)print();
    if(check(last->tid)==0){
        return rr_getNext();
    }
    if(last->exitCode!=INT_MAX)return rr_getNext();
    return tid;
}

static void die (const char *errorMessage){
    fprintf(stderr,"%s",errorMessage);
    exit(-1);
}

static tcb_t * getTCBbyTID(int tid){
    if(nextThread == NULL) return NULL;
    tcb_t *curr = nextThread;
    while(curr->tid != tid){
        if(curr->nextInQ == NULL) return NULL;
        curr = curr->nextInQ;
    }
    return curr;
}

static void print(){
    tcb_t *curr = nextThread;
    while(curr->nextInQ!=NULL && curr != NULL){
        printf("tid %d -->", curr->tid);
        curr = curr->nextInQ;
    }
    printf("%d\n",curr->tid);
}