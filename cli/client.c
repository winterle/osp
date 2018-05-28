#include "client.h"
#include <sys/stat.h>
#define PORT 9000
#define HOST "127.0.0.1"

void cd(){
	int rc;
	if((rc = chdir("cli") != 0)) {
		printf("New directory could not be set!\n");
	}
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

	//TO CHANGE THE DIR TO WOrking Dir form cli
	cd();
	//TO SHOW THE DIR
	/*char dir[300];
	if (getcwd(dir, sizeof(dir)) != NULL)
	fprintf(stdout, "CLIENT: Current working dir: %s\n", dir);
	*/
	while(strcmp(lineBuffer,"logout")!=0){

		char c;
		int i =0;

		while((c=getchar())!=10&&c!='\n'){
			lineBuffer[i]=c;
			i++;
		}
		i=0;
		//START PUT TASK
			if(strstr(lineBuffer, "put")!=NULL){
				char filename[196];

				for (int j = 0; j <196 ; ++j) {
					filename[j]=lineBuffer[4+j];
				}

				FILE *fp = fopen(filename, "r");

				if(fp==NULL){
					printf("Client: Datei nicht gefunden\n");
					continue;

				}else {
				if(write(cli, lineBuffer, 256)<0){
					die("Client: Could not send put task.");
				}
					struct stat st;
					stat(filename, &st);
					int file_size;
					file_size = st.st_size;
					bzero(buf,256);
					sprintf(buf, "%d", file_size);
					int l = strlen(buf);
					if (write(cli, buf, l) < 0)
						die("Couldn't send message");
					bzero(buf,256);

					int block_size;

					while(strstr(buf, "rdy")==NULL){
						if(read(cli, buf, sizeof(buf))<0)
							die("Client: Can not read rdy.\n");
					}
					while ((block_size = fread(buf, sizeof(char), 256, fp)) > 0) {

						if (send(cli, buf, block_size, 0) < 0) {
							die("Client: Can not send file.\n");
						}
						bzero(buf, 256);

					}
					fclose(fp);
					continue;
				}
			}
		//END PUT TASK

		//START GET TASK
		if(strstr(lineBuffer, "get")!=NULL) {

			int block_size;

			char filename[196];

			for (int j = 0; j < 196; ++j) {
				filename[j] = lineBuffer[4 + j];
			}

			FILE *fp = fopen(filename, "w");

			if(write(cli, lineBuffer, 256)<0){
				die("Client: Could not send get task.");
			}
			bzero(buf,256);
			if(read(cli, buf, sizeof(buf))<0)
				die("Client: Can not receive msg.\n");

			int file_size=atoi(buf);
			strcpy(buf,"rdy");

			if (write(cli, buf, 256) < 0)
				die("Couldn't send message");

			bzero(buf,256);

			while((block_size = recv(cli, buf, 256, 0))>0){

				file_size=file_size-block_size;
				if(block_size<0)
				{
					die("Client: Daten konnten nicht erhalten werden.\n");
				}

				int write_size = fwrite(buf, sizeof(char), block_size, fp);

				if(write_size < block_size)
				{
					die("Client: File write failed on server.\n");
				}else if(file_size<=0) {
					fclose(fp);
					break;
				}
				bzero(buf,256);
			}
			continue;
		}
		//END GER TASK

		//if(write(cli, lineBuffer, 256)<0){
		//	die("Client: Can not send msg.\n");
		//}

		//bzero(lineBuffer,256);
			//bzero(buf,256);
		//if(read(cli, buf, sizeof(buf))<0){
		//	die("Client: Can not receive msg.\n");
		//}
		//printf("[recv] %s",  buf);
	}
	printf("Client all closed\n");
	close(cli);
	printf("Client Exit\n");
	exit(0);
	return 0;
}
