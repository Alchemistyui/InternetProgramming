#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define FDSIZE      256
#define EPOLLEVENTS 20

static void handle_accpet(int epollfd, int listenfd) {
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (SA*) &cliaddr, &clilen);

    printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
}

static void do_read(int epollfd, int fd)
{
    // int nread;
    char buf[MAXLINE];

    read(fd, buf, BUFFERSIZ);
}

static void do_write(int epollfd, int fd)
{
    // int nwrite;
    char buf[MAXLINE];

    write(fd, buf, strlen(buf));  
}


int main(int argc, char* argv[])
{
    int listenFd, connFd;
    pid_t childPid;
    socklen_t cliLen;
    struct sockaddr_in cliAddr, servAddr;

    struct epoll_event ev,events[EPOLLEVENTS];
    //生成用于处理 accept 的 epoll 专用的文件􏰀述符
    epfd=epoll_create(FDSIZE);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr));

    listen(listenfd, LISTENQ);

    nfds=epoll_wait(epfd,events,EPOLLEVENTS,500);
    //处理所发生的所有事件
    for(i=0;i<nfds;++i)
    {
        //如果新监测到一个 SOCKET 用户连接到了绑定的 SOCKET 端口，建立新的连接
        if(events[i].data.fd==listenfd) {
            handle_accpet(epfd, listenfd);
            //注册 ev
            epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            
        }
        //如果是已经连接的用户，并且收到数据，那么进行读入。
        else if(events[i].events&EPOLLIN) {
            do_read(epollfd, events[i].data.fd);
            epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);        
        }
        // 如果有数据发送
        else if(events[i].events&EPOLLOUT) {
            do_write(epollfd, events[i].data.fd);
            sockfd = events[i].data.fd;
            //设置用于读操作的文件􏰀述符
            ev.data.fd=sockfd;
            //设置用于注测的读操作事件
            ev.events=EPOLLIN|EPOLLET;
            //修改 sockfd 上要处理的事件为 EPOLIN
            epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
        }

    }

}

