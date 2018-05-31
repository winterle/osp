#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "ult.h"


static void threadA()
{
	ult_exit(0);
}

static void threadB()
{
	ult_exit(0);
}

static void myInit()
{
	printf("executing the initThread\n");

	int tids[2];//, i, status;
	
	printf("spawn A\n");
	tids[0] = ult_spawn(threadA);
    printf("Tid of threadA = %d\n",tids[0]);
	printf("spawn B\n");
	tids[1] = ult_spawn(threadB);
    printf("Tid of threadB = %d\n",tids[1]);

	/*
	for (i = 0; i < 2; ++i)
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
	*/
	ult_exit(0);

}

int main()
{
	ult_init(myInit);
	return 0;
}


