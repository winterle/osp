
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void die(const char* msg)
{
	fputs(msg, stderr);
	putc('\n', stderr);
	exit(-1);
}

int main()
{
	struct sockaddr_in srv_addr, cli_addr;
	socklen_t sad_sz = sizeof(struct sockaddr_in);
	int sfd, cfd;
	ssize_t bytes;
	char buf[256];
	
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(8000);
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
	
	while ((bytes = read(cfd, buf, sizeof(buf))) != 0)
	{
		if (bytes < 0)
			die("Couldn't receive message");
		
		if (write(cfd, buf, bytes) < 0)
			die("Couldn't send message");
	}
	
	close(cfd);
	close(sfd);
	return 0;
}
