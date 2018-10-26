#include "unp.h"

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
            err_sys("read error");
        }

}

int main() {
    int listenFd, connFd;
    pid_t childPid;
    socklen_t cliLen;
    struct sockaddr_in cliAddr, servAddr;

    listenFd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERV_PORT);

    bind(listenFd,(const sockaddr *) &servAddr, sizeof(servAddr));

    listen(listenFd, LISTENQ);

    for(;;){
        cliLen = sizeof(cliAddr);
        connFd = accept(listenFd, &cliAddr, &cliLen);
        str_echo(connFd);
        close(connFd);
    }
}