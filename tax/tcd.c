#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>

/*fixme
 * - verlieren geld durch int.
 * - deadlock (finden/lösen).
 * - geld nicht korrekt.
 */

//debug FLAGS
int Flag_01=0;
int Flag_02=0;
int Flag_03=0;
int Flag_04=0;
int Flag_05=0;
int Flag_06=0;
int Flag_07=0;
int Flag_08=0;
int Flag_09=0;
int Flag_10=0;
int Flag_11=0;


void lockAll();
void unlockAll();

//debug
void DE_if(int Flag, char* msg){
    if(Flag==1){
        printf("%s\n",msg);
    }
}

typedef struct waiter_s{//steven
    int vic;
    struct waiter_s *next;
}waiter;


typedef struct collector_s {
	unsigned int credit;
	unsigned int bookings_in;
	unsigned int bookings_out;
	waiter* queue;
    pthread_mutex_t lock;
}collector_t;

double duration;
int collectors;
unsigned int funds;
int collectorCount;
pthread_mutex_t collectorCountLock;
collector_t *collectorArray;

double time1=0.0, tstart, time2;//steven
int wasprinted=0;//steven



int tryBooking(int,int);


/**
 * prints the current status of the system, requires that all variables are locked before calling!
 * */
void printStats(){
    int total_credit = 0;
    int total_in = 0;
    int total_out = 0;
    for(int i = 0; i < collectors; i++){
        printf("---Collector %d---\n",i);
        printf("bookings_in: %d\n"
        "bookings_out: %d\n"
        "credit: %d\n", collectorArray[i].bookings_in,collectorArray[i].bookings_out,collectorArray[i].credit);
        total_credit += collectorArray[i].credit;
        total_in += collectorArray[i].bookings_in;
        total_out += collectorArray[i].bookings_out;
    }
    printf("/////////////////////////////////////////\n"
           "Total credit: %d (should be %d)\nTotal bookings_in: %d\nTotal bookings_out: %d\n--------",total_credit,funds*collectors,total_in,total_out);
}

/**
 * End all threads if the timelimit is reached
 */
void exitAfterTime(char* line){ //steven
    time1 = clock() - tstart;
    time2 = time1/CLOCKS_PER_SEC;
    if((duration-time2)<=0&&wasprinted==0){
        wasprinted=1;
        printf("%s\n",line);
        lockAll();
        printStats();
        unlockAll();
        exit(0);
    } else{
        if(wasprinted==1){
            unlockAll();
        }
            return;
    }
}

static inline int roll(int sides){
    //exitAfterTime("line 90");//steven
	return rand() / (RAND_MAX + 1.0) * sides;
}

void *collector(void *arg){
    //todo: can avoid the lock by passing the collectorID as argument, only modifying the collectorCount in the initial Thread
    pthread_mutex_lock(&collectorCountLock);
    int collectorID = collectorCount++;
    pthread_mutex_unlock(&collectorCountLock);
    sched_yield();
	printf("Thread %d spawned, collectorID=%d\n",(unsigned int)pthread_self(),collectorID);
    int victim;
    while(1) {
        //exitAfterTime("line 103");//steven
        /* select a thread to steal from */
        //collectors does not require a lock, because the value will never change and therefore the read-access doesn't have to be serialized
        while((victim = roll(collectors)) == collectorID);
        //fixme this works without a deadlock but results in unfair behaviour, since whenever a variable is locked / a collector
        //does not have enough money, no transaction will take place -> solution: push to queue?
        if(!tryBooking(collectorID,victim))continue;
        sched_yield();
    }
}

/**
 * trys to lock the collector_t structs associated with the passed ID's and execute the Booking (from victim to collector)
 * if the credit of the victim is too small, nothing is locked and @return value = 0
 * if at least one lock is already aquired by another thread, nothing is locked and @return = 0
 * if the transaction has been executed @return = 1
 * @param collectorID identifier for the recieving struct
 * @param victimID identifier for the other struct
 * */
int tryBooking(int collectorID, int victimID){
    //exitAfterTime("line 123");//steven
    int c=1;
    int v=1;
    int foundone=0;
    if(Flag_03==1) printf("Collector: %d || Victim: %d Money: %d\n",collectorID, victimID, collectorArray[victimID].credit);
        if (pthread_mutex_trylock(&collectorArray[collectorID].lock) == 0){
            //printf("Lock: %d\n",collectorID);//debug
            c = 0;
        }
            else{ c = 1;}
    if(pthread_mutex_trylock(&collectorArray[victimID].lock)==0) v=0;
    else v=1;

    if(c==0) {
        foundone=1;
        if (collectorArray[victimID].credit < 100||v==1) {
            foundone=0;
            waiter *curr = collectorArray[collectorID].queue;
            if (curr == NULL) {
                collectorArray[collectorID].queue = (waiter *) malloc(sizeof(waiter));
                collectorArray[collectorID].queue->vic = victimID;
                collectorArray[collectorID].queue->next = NULL;
                pthread_mutex_unlock(&collectorArray[collectorID].lock);
                //printf("unlock: %d\n",collectorID);//debug
                pthread_mutex_unlock(&collectorArray[victimID].lock);
                return 0;
            } else {
                while(curr->next!=NULL){
                    curr=curr->next;
                    waiter* new=(waiter*)malloc(sizeof(waiter));
                    new->next=NULL;
                    new->vic=victimID;
                    curr->next=new;
                    pthread_mutex_unlock(&collectorArray[victimID].lock);
                }
                curr = collectorArray[collectorID].queue;
                while(curr!=NULL){
                    if(pthread_mutex_trylock(&collectorArray[curr->vic].lock)==0){
                        if (collectorArray[curr->vic].credit >= 100) {
                            foundone=1;
                            victimID=curr->vic;
                            break;
                        }
                    }
                    curr=curr->next;
                }
                if(curr==NULL){
                    return 0;
                }
            }
        }
        }

            if (c == 0&&foundone==1) {
                int rest = 0;
                int abzug = 0;
                if (collectorArray[victimID].credit % 2 == 1) rest = 1;
                if (collectorArray[victimID].credit / 2 < 100) {
                    abzug = 100;
                } else {
                    abzug = (collectorArray[victimID].credit / 2) + rest;
                }
                DE_if(Flag_02, "pthread_mutex_trylock(&collectorArray[collectorID].lock) == 0");//debug
                collectorArray[collectorID].bookings_in++;
                collectorArray[victimID].bookings_out++;
                collectorArray[collectorID].credit += abzug;
                collectorArray[victimID].credit -= abzug;
                pthread_mutex_unlock(&collectorArray[victimID].lock);
                pthread_mutex_unlock(&collectorArray[collectorID].lock);
                //printf("unlock: %d\n",collectorID);//debug

                return 1;
            }
    pthread_mutex_unlock(&collectorArray[victimID].lock);
    pthread_mutex_unlock(&collectorArray[collectorID].lock);
        return 0;
    }

/*locks all mutexes*/
void lockAll(){
    //exitAfterTime("line 154");//steven
    DE_if(Flag_01,"lockALL"); //debug
    for(int i = 0; i < collectors; i++){
        pthread_mutex_lock(&collectorArray[i].lock);
    }
}
/*unlocks all mutexes*/
void unlockAll(){
    //exitAfterTime("line 162");//steven
    DE_if(Flag_01,"unlockALL"); //debug
    for(int i = 0; i < collectors; i++){
        pthread_mutex_unlock(&collectorArray[i].lock);
    }
}

void *shell(void *arg){

    //char buf[50];
    while(1){
        time1 = clock() - tstart;
        time2 = time1/CLOCKS_PER_SEC;
        if((duration-time2)<=0) {
            lockAll();
            printStats();
            unlockAll();
            exit(0);
        }else{
            sched_yield();
        }
    }
    DE_if(Flag_11,"shell end");//debug
    /* returning 0 implicitly calls pthread_exit(0) */
    return 0;
}

/* initializes all the structs and mutexes/spawnes the desired amount of threads and the shell-thread
 * then waits for termination of the shell thread, deallocates resources and exits
 * */
void init(){
    time1=0.0;//steven
    tstart = clock();//steven

	collectorArray = malloc(collectors * sizeof(collector_t));
	collectorCount = 0;

    /* initialize mutexes before starting any threads */
    if(pthread_mutex_init(&collectorCountLock,NULL))exit(-1);//todo handle error
    for(int i = 0; i < collectors; i++){
        if(pthread_mutex_init(&collectorArray[i].lock,NULL))exit(-1);//todo handle error
        collectorArray[i].queue=NULL;
    }

	/* set the scheduling policy to round robin whilst leaving all other attributes as default values*/
	pthread_attr_t attr;
	if(pthread_attr_init(&attr))exit(-1);//todo handle error
	if(pthread_attr_setschedpolicy(&attr,SCHED_RR))exit(-1);//todo handle error

    /* spawn the shell */
	pthread_t newThread;
    pthread_t shellThread;
    if(pthread_create(&shellThread,&attr,&shell,NULL))exit(-1);//todo handle error
	for(int i = 0; i < collectors; i++){
		collectorArray[i].credit = funds;
		collectorArray[i].bookings_in = collectorArray[i].bookings_out = 0;
		if(pthread_create(&newThread,&attr,&collector,NULL))exit(-1);//todo handle error
        if(pthread_detach(newThread))exit(-1);//todo handle error

	}
	/* waiting for the shell to exit, then free the Array and exit */
    //fixme freeing the array whilst the other threads still work on those values might cause invalid reads/writes?
	pthread_join(shellThread,NULL);
    for(int i = 0; i < collectors; i++){
        pthread_mutex_destroy(&collectorArray[i].lock);
    }
    free(collectorArray);
	exit(0);
}

int main(int argc, const char* argv[])
{
	duration = 10; // default duration in seconds
	collectors = 5;  // default number of tax collectors
	funds = 300;     // default funding per collector in Euro
	
	// allow overriding the defaults by the command line arguments
	switch (argc)
	{
	case 4:
		duration = atof(argv[3]);
		/* fall through */
	case 3:
		funds = (unsigned int)atoi(argv[2]);
		/* fall through */
	case 2:
		collectors = atoi(argv[1]);
		/* fall through */
	case 1:
		printf(
			"Tax Collectors:  %d\n"
			"Initial funding: %d EUR\n"
			"Duration:        %g s\n",
			collectors, funds, duration
		);
		break;
		
	default:
		printf("Usage: %s [collectors [funds [duration]]]\n", argv[0]);
		return -1;
	}

	init();
	return 0;
}
