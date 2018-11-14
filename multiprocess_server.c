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
     
    printf("str echo!!\n");
    time(&now);
    tm_now = localtime(&now);
    datetime = asctime(tm_now);
     
    printf("now datetime: %s\n", datetime);
     
    return ; 
}

int main(int argc, char *argv[]) {
    // str_echo(1);

    int listenfd, connectfd;
    // 注意不是size_t
    socklen_t cliLen;
    struct sockaddr_in cliAddr, servAddr;
    int childpid;

    signal(SIGCHLD, sig_chld);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (SA *) &servAddr, sizeof(servAddr));
    // printf("1\n");
    listen(listenfd, LISTENQ);
    // printf("2\n");
    for(;;) {
        // printf("3\n");
        cliLen = sizeof(cliAddr);
        // printf("4\n");
        connectfd = accept(listenfd, (SA *) &cliAddr, &cliLen);
        // printf("5\n");
        childpid = fork();
        // printf("6\n");
        if(childpid == 0) {
            close(listenfd);
            str_echo(connectfd);
            close(connectfd);
            return 0;
        }
        close(connectfd);
        return 0;
    }
}

// gcc -o multiprocess_server multiprocess_server.c -lpthread
// ./multiprocess_server &

