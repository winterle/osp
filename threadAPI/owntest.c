#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include "ult.h"

ssize_t result;
ssize_t byte;
int exitflag=0;
double time1=0.0, tstart, time2;

int FLAG_A=0;
int FLAG_B=0;
int FLAG_M=0;

static void threadA()
{
    if(FLAG_A==1) printf("hello, here is threadA\n");
    srand(time(NULL));

    time1=0.0;
    tstart = clock();

    char buf [1];

        int randomData = open("/dev/random", O_RDONLY);
        int nullData = open("/dev/null", O_WRONLY);
        if (randomData < 0 || nullData < 0) {
            fprintf(stderr,"ThreadA: Stream error");
            ult_exit(-1);
        } else {
            while (1) {

                time1 = clock() - tstart;

                if(FLAG_A==1) printf("ThreadA: read\n");
                result = ult_read(randomData, buf, sizeof(buf));
                byte= byte + result;
                int x = write(nullData, buf, sizeof(buf));
                if(x){}

                if(FLAG_A==1) printf("read Bytes: %ld\n",result);

                if (exitflag == 1) {
                    printf("ThreadA: Exitflag was set\n");
                    break;
                }
                ult_yield();
                if(FLAG_A==1) printf("while next\n");
            }
            ult_exit(17);
        }
}

static void threadB()
{
    char line [100];
    double durchsatz;

    if(FLAG_B==1) printf("hello, here is threadB\n");

        while(1) {
        bzero(line,100);
        ult_read(0, line, 100);

        if (strcmp(line, "stats\n")==0) {
            printf("Anzahl an bereits kopierten Bytes: %ld\n", byte);
            time2 = time1/CLOCKS_PER_SEC;
            printf("Zeit seit Start des Threads = %f sekunden\n", time2);
            durchsatz = byte/time2;
            printf("Durchsatz: %f\n",durchsatz);
            printf("Shell>");
            fflush(stdout);
            bzero(line,100);
            if(FLAG_B==1) printf("ThreadB \"Stats\", ult_yield()\n");
            ult_yield();
            if(FLAG_B==1) printf("ThreadB: ult_yield was make\n");
            continue;
        }
        if (strcmp(line, "exit\n")==0) {
            printf("ThreadB: Exitflag was set\n");
            exitflag=1;
            break;
        }
        if(strcmp(line,"")!=0) {
            printf("Dont know this Order [%s] :( \n", line);
            printf("Shell>");
            fflush(stdout);
            bzero(line, 100);
            ult_yield();
        }
    }
    if(FLAG_B==1) printf("Shell exit.\n");
    ult_exit(3);
}

static void myInit()
{
    if(FLAG_M==1) printf("executing the initThread\n");

    int tids[2], i, status;

    if(FLAG_M==1) printf("spawn A\n");
    tids[0] = ult_spawn(threadA);
    if(FLAG_M==1) printf("Tid of threadA = %d\n",tids[0]);
    if(FLAG_M==1) printf("spawn B\n");
    tids[1] = ult_spawn(threadB);
    printf("Shell>");
    if(FLAG_M==1) printf("Tid of threadB = %d\n",tids[1]);

    for (i = 0; i < 2; ++i)
    {
        if(FLAG_M==1) printf("waiting for tids[%d] = %d\n", i, tids[i]);
        fflush(stdout);


        if (ult_join(tids[i], &status) < 0)
        {
            fprintf(stderr, "join for %d failed\n", tids[i]);
            ult_exit(-1);
        }

        if(FLAG_M==1) printf("(status = %d)\n", status);
        fflush(stdout);

    }
    ult_exit(0);


}

int main()
{
    ult_init(myInit);
    return 0;
}