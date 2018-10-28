#include "unp.h"
// #include <stdio.h>
// #include <unistd.h>

// int writen(int fd, const void *vptr, size_t n)
// {
//     size_t          nleft = n;  //writen函数还需要写的字节数
//     ssize_t        nwrite = 0; //write函数本次向fd写的字节数
//     const char*    ptr = vptr; //指向缓冲区的指针

//     while (nleft > 0)
//     {
//         if ((nwrite = write(fd, ptr, nleft)) <= 0)
//         {
//             if (nwrite < 0 && EINTR == errno)
//                 nwrite = 0;
//             else
//                 return -1;
//         }
//         nleft -= nwrite;
//         ptr += nwrite;
//     }
//     return n;
// }

void str_echo(int connFd){
    ssize_t n;
    char buf[MAXLINE];

    again:
        while((n = read(connFd, buf, MAXLINE)) > 0){
            writen(connFd, buf, n);
        }
        // EINTR错误(被中断的系统调用)
        if(n < 0 && errno == EINTR){
            goto again;
        }
        else if(n < 0){
            // printf("read error\n");
            // exit(1);
            err_sys("read error");
        }

}

int main(int argc, char **argv) {
    int listenFd, connFd;
    pid_t childPid;
    socklen_t cliLen;
    struct sockaddr_in cliAddr, servAddr;

    listenFd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERV_PORT);

    bind(listenFd,(SA *) &servAddr, sizeof(servAddr));

    listen(listenFd, LISTENQ);

    for(;;){
        cliLen = sizeof(cliAddr);
        connFd = accept(listenFd,(SA *) &cliAddr, &cliLen);
        str_echo(connFd);
        close(connFd);
    }
}

// gcc -o echo_server echo_server.c -lunp

// ./echo_server &
// ./echo_client 127.0.0.1

