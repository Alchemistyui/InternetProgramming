// #include <sys/types.h>
#include "unp.h"
#include <stdio.h>
#include <time.h> 

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child process %d exit.\n", pid);
        return ;
    }
}

void str_echo(int connectfd) {
    time_t now;
    struct tm *tm_now;
    char *datetime;
     
    time(&now);
    tm_now = localtime(&now);
    datetime = asctime(tm_now);
     
    printf("now datetime: %s\n", datetime);
     
    return ; 
}

int main(int argc, char *argv[]) {
    // str_echo(1);
    int listenfd, connectfd;
    size_t cliLen;
    struct sockaddr_in cliAddr, servAddr;

    signal(SIGCHID, sig_chld);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (SA *) &servAddr, sizeof(servAddr));

    listen(listenfd, LISTENQ);

    for(;;) {
        cliLen = sizeof(cliAddr);

        connectfd = accept(listenfd, (SA *) &cliAddr, &cliLen);
        if((childpid = fork()) == 0) {
            close(listenfd);
            str_echo(connectfd);
            close(connectfd);
            return 0;
        }
        close(connectfd);
        return 0;
    }
}