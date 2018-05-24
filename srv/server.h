#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#endif

static void die(const char* msg) {

     fputs(msg, stderr);
     putc('\n', stderr);
     exit(-1);

}

 struct sockaddr_in srv_addr, cli_addr;
 socklen_t sad_sz = sizeof(struct sockaddr_in);
 int sfd, cfd;
 ssize_t bytes;
 char buf[256];