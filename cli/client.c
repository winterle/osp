#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/stat.h>

#define PORT 9000
#define HOST "127.0.0.1"


void get (int cfd, char* buf_in ){

    int block_size;

    char filename[1020];

    for (int j = 0; j < 1020; ++j) {
        filename[j] = buf_in[4 + j];
    }

    FILE *fp = fopen(filename, "w");

    if(write(cfd, buf_in, 1024)<0){
        printf("Client: Could not send get task.\n");
    }
    bzero(buf_in,1024);
    if(read(cfd, buf_in, sizeof(buf_in))<0)
        printf("Client: Can not receive msg.\n");

    int file_size=atoi(buf_in);
    strcpy(buf_in,"rdy");

    if (write(cfd, buf_in, 1024) < 0)
        printf("Couldn't send message\n");

    bzero(buf_in,1024);

    while((block_size = recv(cfd, buf_in, 1024, 0))>0){

        file_size=file_size-block_size;
        if(block_size<0)
        {
            printf("Client: Daten konnten nicht erhalten werden.\n");
        }

        int write_size = fwrite(buf_in, sizeof(char), block_size, fp);

        if(write_size < block_size)
        {
            printf("Client: File write failed on server.\n");
        }else if(file_size<=0) {
            fclose(fp);
            break;
        }
        bzero(buf_in,1024);
    }
    return;
}

void put (int cfd, char* buf_out ){
    char filename[1020];

    for (int j = 0; j <1020 ; ++j) {
        filename[j]=buf_out[4+j];
    }

    FILE *fp = fopen(filename, "r");

    if(fp==NULL){
        printf("Client: Datei nicht gefunden\n");
        return;

    }else {
        if(write(cfd, buf_out, 1024)<0){
            printf("Client: Could not send put task.\n");
        }
        struct stat st;
        stat(filename, &st);
        int file_size;
        file_size = st.st_size;
        bzero(buf_out,1024);
        sprintf(buf_out, "%d", file_size);
        int l = strlen(buf_out);
        if (write(cfd, buf_out, l) < 0)
            printf("Couldn't send message\n");
        bzero(buf_out,1024);

        int block_size;

        while(strstr(buf_out, "rdy")==NULL){
            if(read(cfd, buf_out, sizeof(buf_out))<0)
                printf("Client: Can not read rdy.\n");
        }
        while ((block_size = fread(buf_out, sizeof(char), 1024, fp)) > 0) {

            if (send(cfd, buf_out, block_size, 0) < 0) {
                printf("Client: Can not send file.\n");
            }
            bzero(buf_out, 1024);

        }
        fclose(fp);
        return;
    }
}

void cd_wd(){
    int rc;
    if((rc = chdir("cli") != 0)) {
        printf("New directory could not be set!\n");
    }
}

int main()
{
    cd_wd();
    struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr.s_addr = inet_addr(HOST)
    };
    char buf_in[1024];
    int buf_out_len ,buf_in_len;
    char buf_out[1024];

    int cfd;

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(-1);

    if (connect(cfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        exit(-1);

    printf("Connected to server, shell instance ready:\n");

    cd_wd();
    while(1){
        ioctl(cfd,FIONREAD,&buf_in_len);

        if(buf_in_len>0){//there is some data to be read
            buf_in_len = read(cfd,buf_in,sizeof(buf_in));
            buf_in[buf_in_len] = '\0';
            printf("%s",buf_in);
        }
        ioctl(STDIN_FILENO,FIONREAD,&buf_out_len);
        if(buf_out_len > 0){
            if(fgets(buf_out,sizeof(buf_out),stdin) == NULL)fprintf(stderr, "could not read (fgets)\n");

            if(strstr(buf_in,"put")!=NULL){
                put(cfd, buf_in);
            }
            if(strstr(buf_in,"get")!=NULL){
                get(cfd,buf_in);
            }

            buf_out[buf_out_len] = '\0';
            if(write(cfd,buf_out,buf_out_len)<0)printf("C-[send]FAILED\n");
            buf_out_len = 0;
        }
    

}}
