#include "unp.h"
// #include <std.h>

int main(int argc, char** argv) {
    int i, maxfd, maxi, connectfd, listenfd, sockfd;  
    socklen_t clilen;
    struct sockaddr_in servaddr,cliaddr;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];

    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd,(SA *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    maxi = -1;
    maxfd = listenfd;

    for(i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for(;;) {
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);

            connectfd = accept(listenfd, (SA *) &cliaddr, &clilen);

            printf("new client: %s, port : %d\n", 
                // inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL)
                inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

            for(i = 0; i < FD_SETSIZE; i++) {
                if(client[i] < 0) {
                    client[i] = connectfd;
                    break;
                }
            }

            if(i == FD_SETSIZE) {
                err_quit("client number over set size");
            }

            FD_SET(connectfd, &allset);
            if(connectfd > maxfd) {
                maxfd = connectfd;
            }
            if(i > maxi) {
                maxi = i;
            }
            if(--nready <= 0) {
                continue;
            }

        }

        for(i = 0; i <= maxi; i++) {
            if((sockfd = client[i]) < 0) {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) {
                if((n = read(sockfd, buf, MAXLINE)) == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else {
                    write(sockfd, buf, n);
                }
                if(--nready <= 0) {
                    break;
                }
            }
        }

    }



}















