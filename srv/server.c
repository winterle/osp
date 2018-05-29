#include "server.h"
#include <sys/stat.h>
#define PORT 9000

void cd(){
    int rc;
    if((rc = chdir("srv") != 0)) {
        printf("New directory could not be set!\n");
    }
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

    //TO CHANGE THE DIR TO Working Dir form srv
    cd();
    //TO SHOW THE DIR
    /*char dir[300];
    if (getcwd(dir, sizeof(dir)) != NULL)
    fprintf(stdout, "SERVER: Current working dir: %s\n", dir);
    */

	while ((bytes = read(cfd, buf, sizeof(buf)))) {

	    //START PUT TASK
        if(strstr(buf, "put")!=NULL) {

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

            while((block_size = recv(cfd, buf2, 256, 0))>0){

                file_size=file_size-block_size;

                if(block_size<0)
                {
                    die("Server: Daten konnten nicht erhalten werden.\n");
                }

                int write_size = fwrite(buf2, sizeof(char), block_size, fp);

                if(write_size < block_size)
                {
                    die("Server: File write failed on server.\n");
                }else if(file_size<=0) {
                    fclose(fp);
                    break;
                }

                bzero(buf2,256);
            }
            continue;
        }
        //END PUT TASK

        //START GET TASK
        if(strstr(buf, "get")!=NULL){

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

		//if (bytes < 0) {
          //  die("Couldn't receive message");
        //}

		//if (write(cfd, buf2, bytes) < 0)
		//	die("Couldn't send message");
	}
	printf("Server all closed\n");
	close(cfd); close(sfd);

	/*while (1)
	{
		printf("[srv]: idle\n");
		sleep(1);
	}
	*/
	return 0;
}
