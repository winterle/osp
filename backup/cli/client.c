#include "client.h"

#define PORT 9000
#define HOST "127.0.0.1"


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

	for (int j = 0; j < 256; ++j) {
		lineBuffer[j]='\0';
	}

	cli = create_socket();

	if(connect(cli,(struct sockaddr*) &addr, sizeof(addr))< 0){
		die("Can not connect to socket");
	}

	while(strcmp(lineBuffer,"logout")!=0){
		char c;
		int i =0;

		while((c=getchar())!='\n'&&i!=256){
			lineBuffer[i]=c;
			i++;
		}
        lineBuffer[i]=10;
		if(write(cli, lineBuffer, 256)<0){
			die("Can not send msg.\n");
		}
		//for (int j = 0; j < 256; ++j) {
		//	lineBuffer[i]='\0';
		//}
		if(read(cli, buf, sizeof(buf))<0){
			die("Can not receive msg.\n");
		}
		printf("[recv] %s",  buf);
	}
	close(cli);
	printf("Client Exit\n");
	exit(0);
	return 0;
}
