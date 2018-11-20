#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define FDSIZE      256
#define EPOLLEVENTS 20
#define BUFFERSIZE    1024
#define MAXLINE    1024
#define PORT        9877
#define LISTENQ     1024

static void handle_accpet(int epollfd, int listenfd) {
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
}

static void do_read(int epollfd, int fd)
{
    // int nread;
    char buf[MAXLINE];
    int nread;
    nread = read(fd, buf, BUFFERSIZE);
    if (nread == -1)
    {
        perror("read error:");
    }
    else if (nread == 0)
    {
        fprintf(stderr, "client close.\n");

    }
    else
    {
        printf("read message is : %s", buf);

    }
}

static void do_write(int epollfd, int fd)
{
    // int nwrite;
    char buf[MAXLINE];
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1)
    {
        perror("write error:");

    }
}


int main(int argc, char* argv[])
{
    int listenfd, connfd, epfd, sockfd;
    int i, nfds;
    pid_t childPid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    struct epoll_event ev,events[EPOLLEVENTS];
    

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    // printf("1\n");

    
    // printf("2\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    // printf("3\n");
    bind(listenfd,(struct sockaddr *)&servaddr, sizeof(servaddr));
    // printf("4\n");
    listen(listenfd, LISTENQ);
    // printf("44\n");

    //生成用于处理 accept 的 epoll 专用的文件􏰀述符
    epfd=epoll_create(FDSIZE);

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    
    for (;;) {
        nfds=epoll_wait(epfd,events,EPOLLEVENTS, 500);
    //     if ((nfds = epoll_wait(epfd, events, EPOLLEVENTS, -1)) == -1) {
    //         perror("epoll_pwait");
    //         exit(1);
    //     }

        // printf("5\n");
        //处理所发生的所有事件
        for(i=0;i<nfds;i++)
        {
            printf("55\n");
            sockfd = events[i].data.fd;
            //如果新监测到一个 SOCKET 用户连接到了绑定的 SOCKET 端口，建立新的连接
            if(sockfd==listenfd) {
                printf("6\n");
                handle_accpet(epfd, listenfd);
                //注册 ev
                printf("7\n");
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
                
            }
            //如果是已经连接的用户，并且收到数据，那么进行读入。
            else if(events[i].events&EPOLLIN) {
                printf("in\n");
                do_read(epfd, sockfd);
                epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd,&ev);        
            }
            // 如果有数据发送
            else if(events[i].events&EPOLLOUT) {
                printf("out\n");
                do_write(epfd, sockfd);
                // sockfd = events[i].data.fd;
                //设置用于读操作的文件􏰀述符
                ev.data.fd=sockfd;
                //设置用于注测的读操作事件
                ev.events=EPOLLIN|EPOLLET;
                //修改 sockfd 上要处理的事件为 EPOLIN
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }

        }
}
    close(epfd);

}

