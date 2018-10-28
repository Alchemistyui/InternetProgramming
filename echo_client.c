#include "unp.h"

void str_cli(FILE* stdin, int socketFd){
    char sendLine[MAXLINE], recvLine[MAXLINE];

    while(fgets(sendLine, MAXLINE, stdin) != NULL){
        write(socketFd, sendLine, sizeof(sendLine));

        if(readline(socketFd, recvLine, MAXLINE) == 0){
            err_quit("readline == 0");
        }
        fputs(recvLine, stdout);
    }
}


int main(int argc, char **argv){
    int socketFd;
    struct sockaddr_in servAddr;

    if (argc != 2){
        err_quit("argc != 2");
    }

    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servAddr.sin_addr);

    connect(socketFd, (SA *) &servAddr, sizeof(servAddr));

    str_cli(stdin, socketFd);

    exit(0);
}

// gcc -o echo_client echo_client.c -lunp