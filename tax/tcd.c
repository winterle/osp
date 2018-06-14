#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>


typedef struct collector_s {
	unsigned int credit;
	unsigned int bookings_in;
	unsigned int bookings_out;
    pthread_mutex_t lock;
}collector_t;

double duration;
int collectors;
unsigned int funds;
int collectorCount;
pthread_mutex_t collectorCountLock;
collector_t *collectorArray;

void printStats(){
    //fixme does this require a lock? without a lock, these stats can be inaccurate since the scheduler is preemtive!
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
    printf("Total credit: %d (should be %d)\nTotal bookings_in: %d\nTotal bookings_out: %d\n--------",total_credit,funds*collectors,total_in,total_out);
}

static inline int roll(int sides){
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
        /* select a thread to steal from */
        //fixme does collectors require a lock -> only reading the variable, never writing it...
        while((victim = roll(collectors)) == collectorID);
        //fixme IMPORTANT: does this require a lock to read? this introduces the possibility of a deadlock, might have to use trylock
        while(collectorArray[victim].credit/2 < 100)sched_yield();
        pthread_mutex_lock(&collectorArray[victim].lock);
        pthread_mutex_lock(&collectorArray[collectorID].lock);
        collectorArray[collectorID].bookings_in++;
        collectorArray[victim].bookings_out++;
        collectorArray[collectorID].credit += collectorArray[victim].credit/2;
        collectorArray[victim].credit -= collectorArray[victim].credit/2;
        pthread_mutex_unlock(&collectorArray[victim].lock);
        pthread_mutex_unlock(&collectorArray[collectorID].lock);
        sched_yield();
    }
}

void *shell(void *arg){
    char buf[50];
    while(1){
        if(fgets(buf,sizeof(buf),stdin) == NULL)printf("error at shell\n");//todo handle error
        if(!strcmp(buf,"exit\n"))break;
        if(!strcmp(buf,"stats\n"))printStats();
    }
    return 0;
}

void init(){
	collectorArray = malloc(collectors * sizeof(collector_t));
	collectorCount = 0;

    /* initialize mutexes before starting any threads */
    if(pthread_mutex_init(&collectorCountLock,NULL))exit(-1);//todo handle error
    for(int i = 0; i < collectors; i++){
        if(pthread_mutex_init(&collectorArray[i].lock,NULL))exit(-1);//todo handle error
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
    //fixme freeing the array whilst the other threads still work on those values will cause invalid reads/writes
	pthread_join(shellThread,NULL);
    free(collectorArray);
	exit(0);
}

int main(int argc, const char* argv[])
{
	duration = 2; // default duration in seconds
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
