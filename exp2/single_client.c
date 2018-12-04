#include "unp.h"
// #include<unistd.h> 

void str_cli(FILE* stdin, int socketFd){ 
    char recvLine[MAXLINE];

    if(readline(socketFd, recvLine, MAXLINE) == 0){
        err_quit("readline == 0");
    }
    // printf("%s\n", recvLine);
    fputs(recvLine, stdout);
}

void str_cli2(FILE* stdin, int socketFd){
    char sendLine[MAXLINE], recvLine[MAXLINE];

    while(1){
        if(readline(socketFd, recvLine, MAXLINE) == 0){
            err_quit("readline == 0");
        }
        fputs(recvLine, stdout);
    }
}

int main(int argc, char **argv){
    int socketFd;
    struct sockaddr_in servAddr;

    
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);

    //多重的时候为0，单个为0
    inet_pton(AF_INET, argv[0], &servAddr.sin_addr);

    connect(socketFd, (SA *) &servAddr, sizeof(servAddr));
    
    str_cli(stdin, socketFd);
    

    exit(0);
}

// gcc -o echo_client echo_client.c -lunp