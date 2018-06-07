#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>
#include "ult.h"

ssize_t result;
int exitflag=0;
int stopflag=0;

static void threadA()
{
    printf("hello, here is threadA\n");
        int randomData = open("/dev/random", O_RDONLY);
        int nullData = open("/dev/null", O_WRONLY);
        if (randomData < 0 || nullData < 0) {
            ult_exit(-1);
        } else {
            while (1) {
                result = ult_read(randomData, 0, 0);
                if (result < 0) {
                    // something went wrong
                }
                if ((result % 42) == 0 || stopflag == 1) {
                    stopflag==0;
                    ult_yield();
                    continue;
                }
                if (exitflag == 1) {
                    break;
                }
            }
            ult_exit(17);
        }
}

static void threadB()
{
    char line [100];
    printf("hello, here is threadB\n");
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;



    while(1) {
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            ult_exit(-1);
            perror("select()");
        }
        if (!(FD_ISSET(0, &rfds))) {
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            ult_yield();
            continue;
        }

        if(fgets(line, 100, stdin)==NULL){
            ult_exit(-1);
        }

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        if (strcmp(line, "stats")) {
            stopflag==1;
            printf("Byte: %ld\n", result);
            ult_yield();
            continue;
        }
        if (strcmp(line, "exit")) {
            exitflag==1;
            break;
        }
    }
    ult_exit(0);
}

static void myInit()
{
    printf("executing the initThread\n");

    int tids[2], i, status;

    printf("spawn A\n");
    tids[0] = ult_spawn(threadA);
    printf("Tid of threadA = %d\n",tids[0]);
    printf("spawn B\n");
    tids[1] = ult_spawn(threadB);
    printf("Tid of threadB = %d\n",tids[1]);

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
    ult_exit(0);


}

int main()
{
    ult_init(myInit);
    return 0;
}