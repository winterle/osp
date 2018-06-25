#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#define PORT 9000
int *serverfdPtr;

void die(const char *message);
int newConnectionHandler(int newClientfd);
void handleConnection(int clientfd);
void sigintHandler(int signo);

int main()
{
    signal(SIGINT,sigintHandler);
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t sockaddr_size = sizeof(struct sockaddr_in);
    int serverfd, newClientfd;
    serverfdPtr = &serverfd;

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    srv_addr.sin_addr.s_addr = INADDR_ANY;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Couldn't get a new socket!\n");

    if (bind(serverfd, (struct sockaddr*) &srv_addr, sockaddr_size) < 0) {
        printf("errno = %d\n",errno);
        if(errno == EADDRINUSE)fprintf(stderr,"Address is already in use!\n");
        close(serverfd);
        die("Couldn't bind!\n");
    }
    if (listen(serverfd, 1) < 0)
        die("Listen failed!\n");

    while(1) {
        newClientfd = accept(serverfd, (struct sockaddr *) &cli_addr, &sockaddr_size);
        if (newClientfd < 0) {
            fprintf(stderr, "Couldn't accept the new client, continuing to listen\n");
            continue;
        }
        if(newConnectionHandler(newClientfd) < 0){
            fprintf(stderr, "Couldn't handle the connection (internal forking error), continuing to listen\n");
            close(newClientfd);
        }
    }
    
}
/**
 * @param newClientfd filedescripor of the socket that describes the connection to be handled
 * @returns 0 on success, negative values encountering errors
 * */
int newConnectionHandler(int newClientfd){
    pid_t pid;
    if((pid = fork())<0){
        return -1;
    }
    if(pid == 0){ //is child
        printf("executing shell for new client\n");
        char buf[1];
        buf[0] = '\0';
        setbuf(stdout,buf);
        dup2(newClientfd,STDIN_FILENO);
        dup2(newClientfd,STDOUT_FILENO);
        dup2(newClientfd,STDERR_FILENO);
        close(newClientfd);


        /*char in_buf[1024];
        while(1) {
            if(fgets(in_buf,sizeof(in_buf),stdin) == NULL)in_buf[0] = '\0';
            printf("S-[echo]%s",in_buf);
        }*/
        char *argv[2];
        argv[0] = "./shell";
        argv[1] = NULL;
        execvp("./shell",argv); //TODO: set execution path to where ./shell can be located, not assuming same directory
        die("Couldn't execute shell for new client\n");
    }
    if(pid > 0) {//is parent
        close(newClientfd); //not needed at parent
        return 0;
    }
    return 0;

}

void die(const char *message){
    fputs(message,stderr);
    close(*serverfdPtr);
    exit(-1);
}

void sigintHandler(int signo){
    close(*serverfdPtr);
    exit(-1);
}
