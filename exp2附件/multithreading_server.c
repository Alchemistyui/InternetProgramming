#include "unp.h"
// #include "unpthread.h"
#include <pthread.h>
#include "string.h"
#include <stdio.h>
#include <stdlib.h>  
#include <time.h> 


pthread_mutex_t mutex_lock;

void show_time(int sockfd) {
    time_t now;
    struct tm *tm_now;
    char *datetime;
    char buf[MAXLINE];
     
    time(&now);
    tm_now = localtime(&now);
    datetime = asctime(tm_now);
     
    
    // printf("connectfd: %d\n", connectfd);

    strcpy(buf,datetime);

    // printf("now datetime: %s\n", buf);

    // 为防止多线程同时写加锁
    pthread_mutex_lock(&mutex_lock);
    // if (pthread_mutex_lock(&mutex_lock) != 0){
    //             printf("lock error!\n");
    //     }
    writen(sockfd, buf, MAXLINE);
    pthread_mutex_unlock(&mutex_lock);

    
     
    return ; 
}


void* str_echo2(void *arg) {
    int sockfd = *(int*) arg;

    printf("\n");
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
    

    pthread_mutex_init(&mutex_lock,NULL);
    

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

