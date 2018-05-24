#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#endif

static void die(const char* msg) {
    fputs(msg, stderr);
    putc('\n', stderr);
    exit(-1);
 }
