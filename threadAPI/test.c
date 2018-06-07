#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "ult.h"


static void threadA()
{
    printf("hello, here is threadA\n");
	ult_exit(1);
}

static void threadB()
{
    printf("hello, here is threadB\n");
	ult_exit(2);
}

static void threadC()
{
    ult_exit(3);
}

static void threadD()
{
    ult_exit(4);
}

static void threadE()
{
    ult_yield();
    ult_exit(5);
}
static void threadF()
{
    ult_exit(6);
}
static void threadG()
{
    ult_exit(7);
}

static void threadH()
{

    ult_exit(8);
}

static void myInit()
{
	int tids[8], i, status;
	
	printf("@initThread: Spawn threads [A-H]\n");
	tids[0] = ult_spawn(threadA);
	tids[1] = ult_spawn(threadB);
    tids[2] = ult_spawn(threadC);
    tids[3] = ult_spawn(threadD);
    tids[4] = ult_spawn(threadE);
    tids[5] = ult_spawn(threadF);
    tids[6] = ult_spawn(threadG);
    tids[7] = ult_spawn(threadH);


	for (i = 0; i < 8; ++i)
	{
		printf("waiting for tids[%d] = %d\n", i, tids[i]);
		fflush(stdout);

		if (ult_join(tids[i], &status) < 0)
		{
			fprintf(stderr, "join for %d failed\n", tids[i]);
			ult_exit(-1);
		}
		
		printf("(status = %d)\n", status);

	}
    ult_exit(0);


}

int main()
{
	ult_init(myInit);
	return 0;
}


