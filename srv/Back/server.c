#include "server.h"
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <asm/errno.h>
#include <errno.h>
#define PORT 9000

void cd_serv(){
    int rc;
    if((rc = chdir("srv") != 0)) {
        printf("New directory could not be set!\n");
    }
}

static int split(int* fd){

    int pfd[2][2];
    int p1 =pipe(pfd[0]);
    int p2 =pipe(pfd[1]);

    if(p1==-1||p2==-2){
        printf("Could not create the pipes.\n");
        exit(-1);
    }

    switch (fork()){
        case 0:
            close(pfd[0][1]);
            close(pfd[1][0]);
            fd[0]= pfd[0][0];
            fd[1]= pfd[1][1];
            return 0;

        default:
            close(pfd[1][1]);
            close(pfd[0][0]);
            fd[0]= pfd[1][0];
            fd[1]= pfd[0][1];
            return 1;
    }
}

static void start_shell(int * fd){

    switch (split(fd)){
        case 0:
            //setbuf(stdout, NULL);
            //setbuf(stdin, NULL);
            dup2(fd[0],STDIN_FILENO);
            //dup2(fd[1],STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execlp("./shell", "shell", NULL);
            exit(-1);


    }
}

int newConnectionHandler(int cfd){
    pid_t pid;
    pid = fork();
    if(pid<0){
        return -1;
    }
    if(pid == 0) { //is child
        printf("executing shell for new client\n");
        return 1;
    }
    if(pid>0) {
        return 0;
    }
    return -1;
}



int main() {
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    srv_addr.sin_addr.s_addr = INADDR_ANY;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Couldn't open the socket");
    if (bind(sfd, (struct sockaddr *) &srv_addr, sad_sz) < 0)
        die("Couldn't bind socket");
    if (listen(sfd, 8) < 0)
        die("Couldn't listen to the socket");

    while(1) {
        cfd = accept(sfd, (struct sockaddr *) &cli_addr, &sad_sz);
        if (cfd < 0)
            die("Couldn't accept incoming connection");
        int b = newConnectionHandler(cfd);

        if(b==0){
            continue;
        }
        if(b==1){
            break;
        }
    }
    //TO CHANGE THE DIR TO Working Dir form srv
    cd_serv();
    //TO SHOW THE DIR
    /*char dir[300];
    if (getcwd(dir, sizeof(dir)) != NULL)
    fprintf(stdout, "SERVER: Current working dir: %s\n", dir);
    */

    int pipePtr[2];
    start_shell(pipePtr);
	while ((bytes = read(cfd, buf, sizeof(buf)))) {
        if (bytes < 0) {
            die("Couldn't receive message");
        }

	    //START PUT TASK
        if(strstr(buf, "put")!=NULL) {
            printf("start S put\n");
            int block_size;

            char filename[196];

            for (int j = 0; j < 196; ++j) {
                filename[j] = buf[4 + j];
            }

            FILE *fp = fopen(filename, "w");

            bzero(buf,256);
            if(read(cfd, buf, sizeof(buf))<0)
                die("Client: Can not receive msg.\n");

            int file_size=atoi(buf);
            bzero(buf,256);


            strcpy(buf,"rdy");
            if (write(cfd, buf, bytes) < 0)
                die("Couldn't send message");
            bzero(buf,256);

            while((block_size = recv(cfd, buf, 256, 0))>0){

                file_size=file_size-block_size;

                if(block_size<0)
                {
                    die("Server: Daten konnten nicht erhalten werden.\n");
                }

                int write_size = fwrite(buf, sizeof(char), block_size, fp);

                if(write_size < block_size)
                {
                    die("Server: File write failed on server.\n");
                }else if(file_size<=0) {
                    fclose(fp);
                    break;
                }

                bzero(buf,256);
            }
            continue;
        }
        //END PUT TASK

        //START GET TASK
        if(strstr(buf, "get")!=NULL){
            printf("start S get\n");
            char filename[196];

            for (int j = 0; j <196 ; ++j) {
                filename[j]=buf[4+j];
            }

            FILE *fp = fopen(filename, "r");

            if(fp==NULL){
                printf("Server: Datei nicht gefunden\n");
                continue;

            }else {
                struct stat st;
                stat(filename, &st);
                int file_size;
                file_size = st.st_size;
                bzero(buf,256);
                sprintf(buf, "%d", file_size);

                if (write(cfd, buf, bytes) < 0)
                	die("Couldn't send message");
                bzero(buf,256);
                int block_size;
                while(strstr(buf, "rdy")==NULL){
                    if(read(cfd, buf, sizeof(buf))<0)
                        die("Client: Can not read rdy.\n");
                }
                while ((block_size = fread(buf, sizeof(char), 256, fp)) > 0) {

                    if (send(cfd, buf, block_size, 0) < 0) {
                        die("Server: Can not send file.\n");
                    }
                    bzero(buf, 256);

                }
                fclose(fp);
                continue;
            }
        }
        //END GET TASK
        strcat(buf,"\n");
        if(write(pipePtr[1], buf, bytes)<0) {
            die("P0 dont work");
            close(cfd); close(sfd);
            exit(-1);
        }

        //die naechsten 2 printf zeigen an was wir dem socked geben wuerden und wie gross es ist
        printf("write: %s\n",buf);
        printf("bytes:%ld\n",bytes);

        if(read(pipePtr[0], buf2, sizeof(buf2))<0) {
            die("P1 dont work");
            close(cfd); close(sfd);
            exit(-1);
        }
        fprintf(stdout, "BUFFER: %s",buf2);
        //die naechsten 2 printf zeigen an was wir vom socked lesen wuerden und wie gross es ist
        printf("read: %s\n",buf2);
        printf("buf:%ld\n", sizeof(buf2));
        if (write(cfd, buf2, bytes) < 0)
            die("Couldn't send message");

        bzero(buf2, 256);
        bzero(buf, 256);
	}
	printf("Server all closed\n");
	close(cfd); close(sfd);
	exit(0);

	/*while (1)
	{
		printf("[srv]: idle\n");
		sleep(1);
	}
	*/
	return 0;
}
