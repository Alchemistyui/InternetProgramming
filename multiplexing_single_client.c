#include "unp.h"
// #include<unistd.h> 

void str_cli(FILE* stdin, int socketFd){
   char sendLine[MAXLINE], recvLine[MAXLINE];
    // printf("client0\n");
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

    
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    // printf("socket\n");
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    // printf("emm\n");
    // printf("1 %s\n", argv[0]);
    // printf("2 %s\n", argv[1]);
    // printf("3 %s\n", argv[2]);
    //多重的时候为0，单个为0
    // printf("single\n");
    inet_pton(AF_INET, argv[0], &servAddr.sin_addr);
    // printf("5\n");
    connect(socketFd, (SA *) &servAddr, sizeof(servAddr));
    
    str_cli(stdin, socketFd);
    

    exit(0);
}

// gedit mp_single_client.c
// gcc -o mp_single_client mp_single_client.c -lunp