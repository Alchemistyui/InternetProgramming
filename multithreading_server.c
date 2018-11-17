#include "unp.h"
// #include "unpthread.h"
#include <pthread.h>
#include "string.h"
#include <stdio.h>
#include <time.h> 

void show_time(int sockfd) {
    time_t now;
    struct tm *tm_now;
    char *datetime;
    char buf[MAXLINE];
     



    //printf("str echo!!\n");
    time(&now);
    tm_now = localtime(&now);
    datetime = asctime(tm_now);
     
    
    // printf("connectfd: %d\n", connectfd);
    // n = read(connFd, buf, MAXLINE);
    // buf = datetime;
    strcpy(buf,datetime);
    writen(sockfd, buf, MAXLINE);
    printf("now datetime: %s\n", buf);
     
    return ; 
}

void* str_echo2(void *arg) {
    int sockfd = *(int*) arg;
    // char *buffer = (char*) malloc(BUFFER_SIZE);

    pthread_detach(pthread_self());
    show_time(sockfd);
    close(sockfd);

    return NULL;
}


int main(int argc, char **argv) {
    int listenfd,connectfd;
    socklen_t clientLen;
    struct sockaddr_in cliAddr, servAddr;
    pthread_t pid;

    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (SA *) &servAddr, sizeof(servAddr));

    listen(listenfd, LISTENQ);

    for(;;) {
        clientLen = sizeof(cliAddr);
        connectfd = accept(listenfd, (SA *) &cliAddr, &clientLen);

        pthread_create(&pid, NULL, &str_echo2, &connectfd);

    }
    return 0;
}



// gcc -o multithreading_server multithreading_server.c -lpthread -lunp
// gcc -o multithreading_server multithreading_server.c -pthread -lunp

// ./multithreading_server &














