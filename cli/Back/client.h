#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <asm/errno.h>
#include <errno.h>

#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#endif

char buf[256];

static void die(const char* msg) {
    fputs(msg, stderr);
    putc('\n', stderr);
    exit(-1);
 }
