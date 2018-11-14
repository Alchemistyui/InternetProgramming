#include "unp.h"
// #include<unistd.h> 

void str_cli(FILE* stdin, int socketFd){
    printf("???\n");
    printf("%d\n", socketFd);
}

// void str_cli2(FILE* stdin, int socketFd){
//     char sendLine[MAXLINE], recvLine[MAXLINE];

//     while(fgets(sendLine, MAXLINE, stdin) != NULL){
//         write(socketFd, sendLine, sizeof(sendLine));

//         if(readline(socketFd, recvLine, MAXLINE) == 0){
//             err_quit("readline == 0");
//         }
//         fputs(recvLine, stdout);
//     }
// }

int main(int argc, char **argv){
    int socketFd;
    struct sockaddr_in servAddr;
    
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    printf("emm\n");
    printf("%s\n", argv[1]);
    inet_pton(AF_INET, argv[1], &servAddr.sin_addr);
    printf("5\n");
    connect(socketFd, (SA *) &servAddr, sizeof(servAddr));
    printf("single\n");
    str_cli(stdin, socketFd);
    

    exit(0);
}

// gcc -o echo_client echo_client.c -lunp