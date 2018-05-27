#include "server.h"

#define PORT 9000

int ls(){
    int rc;
    if((rc = chdir("srv") != 0)) {
        printf("New directory could not be set!\n");
    }

    pid_t pid;
    int status;

    if((pid=fork())==0){
        execlp("/bin/ls", "ls", NULL);
        exit(-1);
    }
    wait(&status);
    return WEXITSTATUS(status);
}

int main() {
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    srv_addr.sin_addr.s_addr = INADDR_ANY;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Couldn't open the socket");
    if (bind(sfd, (struct sockaddr *) &srv_addr, sad_sz) < 0)
        die("Couldn't bind socket");
    if (listen(sfd, 1) < 0)
        die("Couldn't listen to the socket");
    cfd = accept(sfd, (struct sockaddr *) &cli_addr, &sad_sz);
    if (cfd < 0)
        die("Couldn't accept incoming connection");

    ls();
    char dir[300];

    if (getcwd(dir, sizeof(dir)) != NULL) {
    fprintf(stdout, "SERVER: Current working dir: %s\n", dir);
    }
	while ((bytes = read(cfd, buf, sizeof(buf)))) {

	    //START PUT TASK
        if(strstr(buf, "put")!=NULL) {
            int block_size;

            printf("buf: %s\n",buf);
            char filename[196];

            for (int j = 0; j < 196; ++j) {
                filename[j] = buf[4 + j];
            }
            printf("filename: %s\n",filename);
            FILE *fp = fopen(filename, "w");


            while((block_size = recv(cfd, buf, 256, 0))>0){
                if(block_size<0)
                {
                    die("Daten konnten nicht erhalten werden.\n");
                }
                printf("wbuf: %s\n",buf);
                int write_size = fwrite(buf, sizeof(char), block_size, fp);

                if(write_size < block_size)
                {
                    die("File write failed on server.\n");
                }else if(block_size<0) {
                    break;
                }
                bzero(buf,256);
            }
            continue;
        }
        //END PUT TASK

		if (bytes < 0) {
            die("Couldn't receive message");
        }

		//if (write(cfd, buf2, bytes) < 0)
		//	die("Couldn't send message");
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
