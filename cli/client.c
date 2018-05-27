#include "client.h"

#define PORT 9000
#define HOST "127.0.0.1"

int ls(){
	int rc;
	if((rc = chdir("cli") != 0)) {
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

int create_socket(){
	int connector = socket(AF_INET, SOCK_STREAM, 0);
	if(connector<0) {
		die("Can not create a socket.");
		return -1;
	}else{
		return connector;
	}
}

int main()
{
	/*int sys = system ("/bin/stty raw");
	if(sys==-1){
		printf("dont work.\n");
	}*/
	struct sockaddr_in addr = {
			.sin_family = AF_INET,
			.sin_port = htons(PORT),
			.sin_addr.s_addr = inet_addr(HOST)
	};

	char lineBuffer[256];
	char buf[256];
	int cli;

	bzero(lineBuffer,256);
	bzero(buf, 256);
	cli = create_socket();

	if(connect(cli,(struct sockaddr*) &addr, sizeof(addr))< 0){
		die("Can not connect to socket");
	}
	sleep(5);
	ls();
	char dir[300];
	if (getcwd(dir, sizeof(dir)) != NULL)
	fprintf(stdout, "CLIENT: Current working dir: %s\n", dir);

	while(strcmp(lineBuffer,"logout")!=0){
		char c;
		int i =0;

		//while((c=getchar())!='\n'&&i!=256){
		//	lineBuffer[i]=c;
		//	i++;
		//}
        //lineBuffer[i]=10;
		while((c=getchar())!=10&&c!='\n'){
			lineBuffer[i]=c;
			i++;
		}
		i=0;
		//START PUT TASK
			if(strstr(lineBuffer, "put")!=NULL){
				printf("linebuffer: %s\n",lineBuffer);

				if(write(cli, lineBuffer, 256)<0){
					die("Could not send put task.");
				}
				char filename[196];

				for (int j = 0; j <196 ; ++j) {
					filename[j]=lineBuffer[4+j];
				}
				printf("filename: %s\n",filename);
				FILE *fp = fopen(filename, "r");

				if(fp==NULL){
					printf("Datei nicht gefunden\n");

				}else {
					int block_size;
					while ((block_size = fread(buf, sizeof(char), 256, fp)) > 0) {

						if (send(cli, buf, block_size, 0) < 0) {
							die("Can not send file.\n");
						}
						bzero(buf, 256);

					}
					printf("finish.\n");
					close(cli);
					//continue;
				}
			}
		//END PUT TASK
		//if(write(cli, lineBuffer, 256)<0){
		//	die("Can not send msg.\n");
		//}
		//for (int j = 0; j < 256; ++j) {
		//	lineBuffer[i]='\0';
		//}
		//if(read(cli, buf, sizeof(buf))<0){
		//	die("Can not receive msg.\n");
		//}
		//printf("[recv] %s",  buf);
	}
	close(cli);
	printf("Client Exit\n");
	exit(0);
	return 0;
}
