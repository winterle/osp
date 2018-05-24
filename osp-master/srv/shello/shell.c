#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <asm/errno.h>
#include <errno.h>

#define SUCCESS_CODE 0
#define ERROR_CODE (-1)

#define ARGV_MAX_SIZE 256

/*

 * problems:
 * when interrupting wait with ^C, there is a memory leak
 * calling wait on non-children results in an endless loop
 * */
/* Structures */

/* Global Variables */

char currDirName[150];
char lineBuffer[200];
char **argvPtr;
char *pipeSign = "|";
char *backgroundSign = "&";
char *exitSign = "exit";
char *waitSign = "wait";
char *cdSign = "cd";

/* Function Declarations */
pid_t run(int argc, char **argv, unsigned short mode);
pid_t runWithPipe(int argc, char **argv, int *pipePtr, unsigned short read);
void execLine();
void getNextLine();
void handleExecution(int argc, char *argv[]);
void intHandler(int signo);
int * getNewPipe();
void cd(char **argv);
void freeArgv(char **argv);
void waitSeveral(char** argv);


int main(void){
    setbuf(stdout, NULL);
    signal(SIGINT,SIG_IGN);
    if(getcwd(currDirName,sizeof(currDirName)) == NULL){
        printf("Path-name too long!");
        exit(ERROR_CODE);
    }
    while(1){
        printf("%s>",currDirName);
        execLine();
    }

}

pid_t run(int argc, char **argv, unsigned short mode){
    pid_t pid;
    pid = fork();

    if(pid<0) return ERROR_CODE;
    if(pid == 0){//is child
        printf("\n");
        execvp(argv[0],argv);
        printf("Didn't execute '%s' : Are you sure this is an executable file?\n",argv[0]);
        exit(ERROR_CODE);
    }
    if(pid > 0){//is parent
        int rc;
        if(mode == 0){
            waitpid(pid,&rc,0);
        }
        else if(mode == 2){ //background mode
            printf("[%d]\n",pid);
        }
        return SUCCESS_CODE;
    }
    return ERROR_CODE;
}

pid_t runWithPipe(int argc, char **argv, int *pipePtr, unsigned short read){ //TODO: catch errors at close() and dup2()
    pid_t pid;
    pid = fork();
    if(pid<0) return ERROR_CODE;
    if(pid == 0){//is child
        printf("\n");
        if(read == 1){
            dup2(pipePtr[0],STDIN_FILENO);
            close(pipePtr[1]);
            close(pipePtr[0]);
        }
        else{
            dup2(pipePtr[1],STDOUT_FILENO);
            close(pipePtr[0]);
            close(pipePtr[1]);
        }
        execvp(argv[0],argv);
        printf("Didn't execute:'%s' Are you sure this is an executable file?\n",argv[0]);
        exit(ERROR_CODE);
    }
    if(pid > 0){
        //currently handles a pipe call as if both were running in the background
        printf("[%d]",pid);
        if(read == 1){
            printf("is reading from pipe\n");
        }
        else {
            printf("is writing to pipe\n");
        }
        return SUCCESS_CODE;
    }
    return ERROR_CODE;
}

void execLine(){
    getNextLine(); //get a new line from stdin (which is then parsed as a set of actions to be executed)
    if(strlen(lineBuffer) == 1){
        return;
    }
    int argc = 0; //argument counter
    char *position = lineBuffer; //internal pointer to current argument
    unsigned int length = 0;
    char *arg = NULL;
    char **argv = malloc(sizeof(char *) * ARGV_MAX_SIZE);
    for(int i = 0; i < sizeof(lineBuffer) && lineBuffer[i]!='\0'; i++){
        length++;
        if(lineBuffer[i] == 32 || lineBuffer[i] == 10){ //if whitespace or newline
            argc++;
            arg = malloc(length+1);
            argv[argc-1] = arg;
            sscanf(position,"%s",arg); //copy next string at position to arg
            position = lineBuffer+i;
            length = 0;
        }
    }
    argv[argc] = NULL;
    /*
     * At this point, int argc is the number of arguments (including pipes, calls etc),
     * all those arguments are stored inside **argv.
     * we can also keep the first argument, since it usually refers to the process name.
     * now we inspect those arguments using handleExecution()
     */
    handleExecution(argc,argv);
}

void getNextLine(){
    if((fgets(lineBuffer,sizeof(lineBuffer),stdin)) == NULL) printf("fgets failed\n");
}
/**
 * @param argc number of arguments inside
 * @param argv array containing pointers to arguments
 * inspects argv and decides what to do:
 * pipe-mode,background-mode,normal-mode,exit-mode,cd-mode or wait-mode
 * */
void handleExecution(int argc, char *argv[]){
    unsigned short mode = 0;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i],pipeSign) == 0){
            if(argv[i+1] == NULL || i==0){
                printf("can't pipe to(from) the void: please enter an executable file to pipe to(from)!\n");
                free(argv);
                return;
            }
            int *pipePtr;
            char **argv2 = malloc(sizeof(char *) * ARGV_MAX_SIZE);
            int argc2 = 0;
            if(argv[i+1]==NULL)return;
            free(argv[i]);
            argv[i] = NULL;
            int c = 0;
            for(int j = i+1; j < argc; j++){
                argv2[c] = argv[j];
                c++;
            }
            argv2[c] = NULL;
            argc2 = argc-i-1;
            argc = i-1;


            if((pipePtr = getNewPipe()) == 0){
                printf("couldn't create a new Pipe, aborting\n");
                return;
            }
            runWithPipe(argc, argv, pipePtr, 0);
            runWithPipe(argc2, argv2, pipePtr, 1);
            freeArgv(argv);
            freeArgv(argv2);
            free(pipePtr);
            return;
        }
        else if(strcmp(argv[i],backgroundSign) == 0){
            free(argv[i]);
            argv[i] = NULL;
            mode = 2;
            run(argc, argv, mode);
            freeArgv(argv);
            return;
        }
        else if(strcmp(argv[i],exitSign) == 0){
            //currently the program will leave zombies behind when calling "exit"
            freeArgv(argv);
            exit(SUCCESS_CODE);
        }
        else if(strcmp(argv[i],waitSign) == 0){
            if(argv[i+1] == NULL){
                freeArgv(argv);
                return;
            }
            waitSeveral(argv);
            freeArgv(argv);
            return;
            /*
            printf("Waiting for process with pid %d to finish\n", atoi(argv[1]));
            int rc = 0;
            waitpid(atoi(argv[1]),&rc,0);
            freeArgv(argv);
            if(WIFEXITED(rc) == 0){
                printf("child didn't exit properly: WIFEXITED was %d\n",WIFEXITED(rc));
                return;
            }
            printf("%d returned %d on exit!\n",atoi(argv[1]),WEXITSTATUS(rc));
            return;
             */
        }
        else if(strcmp(argv[i],cdSign)==0){
            cd(argv);
            freeArgv(argv);
            return;
        }

    }

    run(argc, argv, mode);
    freeArgv(argv);



}
/**
 * examines the signal number and takes according actions
 * @param signo signal number to be examined
 * */
void intHandler(int signo){ //FIXME memory leak: how to free argv from here?
    printf("Signal %d detected, stopping wait!\n",signo);
    freeArgv(argvPtr);  //set global argv in waitSeveral() and we can free here, not very nice tho
    main();
}

/**
 * @return pointer to dynamically allocated array, that is initialized as a new pipe (2 elements)
 * don't forget to free!
 * */
int * getNewPipe(){
    int *pipePtr = malloc(2*sizeof(int));
    if(pipe(pipePtr) != 0){
        return 0;
    }
    return pipePtr;
}
/**
 * @param argv parsed argv array pointing to strings
 * sets the cwd (current working directory)
 * */

void cd(char **argv){
    int rc;
    if((rc = chdir(argv[1])) != 0){
        printf("New directory could not be set!\n");
        return;
    }
    else if(getcwd(currDirName,sizeof(currDirName)) == NULL) printf("Current Directory too long!\n");
}


/**
 * @param argv argv array to free
 * recursively free's the arguments argv points to, then free's argv itself
 * */
void freeArgv(char **argv){
    for(int i = 0; argv[i] != NULL; i++){
        free(argv[i]);
    }
    free(argv);
}

/**
 * waits for one up to eight background processes to return and writes information about termination to stdout
 * @bug waiting for non-background processes (those, that are not children of this process) results in endless loop
 * @param argv the pid's to wait for
 * */
void waitSeveral(char** argv){
    argvPtr=argv;
    signal(SIGINT,intHandler);
    pid_t backgroundProcessPid[8];
    short backgroundProcessCount = 0;
    pid_t returned;
    int status;
    for(int i = 1; argv[i] != NULL; i++){
        backgroundProcessPid[i-1] = (pid_t)strtol(argv[i],NULL,10);
        backgroundProcessPid[i] = 0;
        if(backgroundProcessPid[i-1] == 0){
            printf("Not a valid pid (conversion error using strtol), aborting!\n");
            return;
        }
        backgroundProcessCount++;
    }
    while(backgroundProcessCount)
    {
        returned = wait(&status);
        if(errno == ECHILD){
            printf("no more running children, stopping wait\n");
            return;
        }

        for(int i = 0; backgroundProcessPid[i] != 0; i++){
            if(returned == backgroundProcessPid[i]) {
                backgroundProcessCount--;
                if(WIFEXITED(status) == 0){
                    printf("child [%d] didn't exit properly: WIFEXITED was %d\n",returned,WIFEXITED(status));
                }
                if(WIFSIGNALED(status)){
                    printf("child [%d] exited due to receiving the signal %d\n", returned, WSTOPSIG(status));
                }
                else if(WIFEXITED(status)){
                    printf("child [%d] seems to have terminated by itself and returned %d on exit\n",returned,WEXITSTATUS(status));
                }
            }
        }
    }

}