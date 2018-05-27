#include "server.h"

#define PORT 9000

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
            dup2(fd[0],0);
            dup2(fd[1],1);
            execlp("./srv/shell", "shell", NULL);
            exit(-1);


    }
}


int main()
{
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port=htons(PORT);
	srv_addr.sin_addr.s_addr = INADDR_ANY;

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		 die("Couldn't open the socket");
	if (bind(sfd, (struct sockaddr*) &srv_addr, sad_sz) < 0)
		 die("Couldn't bind socket");
	if (listen(sfd, 1) < 0)
		 die("Couldn't listen to the socket");
	cfd = accept(sfd, (struct sockaddr*) &cli_addr, &sad_sz);
	if (cfd < 0)
		die("Couldn't accept incoming connection");

    int pipePtr[2];
    int x=0;
    start_shell(pipePtr);
	while ((bytes = read(cfd, buf, sizeof(buf)))) {

	    //Das x ist dafür da um zu schauen wie oft wir diese read-schleife durchlaufen
	    x++;
	    printf("X: %d\n",x);
	    //mit dem IN schauen wir was im buf steht nachdem wir was erhalten/gelesen haben
	    printf("IN:%s",buf);
		if (bytes < 0) {
            die("Couldn't receive message");
        }
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
            //die naechsten 2 printf zeigen an was wir vom socked lesen wuerden und wie gross es ist
            printf("read: %s\n",buf2);
            printf("buf:%ld\n", sizeof(buf2));
		if (write(cfd, buf2, bytes) < 0)
			die("Couldn't send message");
	}
	close(cfd); close(sfd);

	/*while (1)
	{
		printf("[srv]: idle\n");
		sleep(1);
	}
	*/
	return 0;
}
