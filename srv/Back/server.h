#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
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
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <asm/errno.h>
#include <errno.h>
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
 char buf2[256];