#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>

#define PORT        9877
#define BUFFERSIZE    1024
#define LISTENQ     1024
#define FDSIZE      256
#define EPOLLEVENTS 20

//创建套接字并进行绑定
static int socket_bind(int port);
//IO多路复用epoll
static void do_epoll(int listenfd);
//事件处理函数
static void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);
//处理接收到的连接
static void handle_accpet(int epollfd, int listenfd);
//读处理
static void do_read(int epollfd, int fd, char *buf);
//写处理
static void do_write(int epollfd, int fd, char *buf);
//添加事件
static void add_event(int epollfd, int fd, int state);
//修改事件
static void modify_event(int epollfd, int fd, int state);
//删除事件
static void delete_event(int epollfd, int fd, int state);


int main(int argc, char** argv) {
    int listenfd;
    listenfd = socket_bind(PORT);
    listen(listenfd, LISTENQ);
    do_epoll(listenfd);
    return 0;
}

static int socket_bind(int port) {
    int listenfd;
    struct sockaddr_in servaddr;

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        err_quit("cerat socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    return listenfd;

}

static void do_epoll(int listenfd) {
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int sum;
    char buf[BUFFERSIZE];

    // 将s所指向的某一块内存中的后n个 字节的内容全部设置为ch指定的ASCII值， 
    // 第一个值为指定的内存地址，块的大小由第三个参数指定
    memset(buf, 0, BUFFERSIZE);

    // 创建一个epoll描述符
    epollfd = epoll_create(FDSIZE);
    // 添加监听描述符的事件
    add_event(epollfd, listenfd, EPOLLIN);

    for (;;) {
        if ((num = epoll_wait(epollfd, events, EPOLLEVENTS, -1)) == -1) {
            err_quit("epoll wait error");
        }
        handle_events(epollfd, events, num, listenfd, buf);
    }
    close(epollfd);

}

static void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf) {
    int i;
    int fd;

    for(i = 0; i<num; i++) {
        // 根据描述符类型和事件类型进行处理
        if ((fd == listenfd) && (events[i].events & EPOLLIN))
            handle_accpet(epollfd, listenfd);
        else if (events[i].events & EPOLLIN)
            do_read(epollfd, fd, buf);
        else if (events[i].events & EPOLLOUT)
            do_write(epollfd, fd, buf);
    }
}

static void handle_accpet(int epollfd, int listenfd) {
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (SA*) &cliaddr, &clilen);

    printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
    //添加一个客户描述符和事件
    add_event(epollfd, clifd, EPOLLIN);
}


static void do_read(int epollfd, int fd, char *buf)
{
    int nread;
    nread = read(fd, buf, BUFFERSIZ);
    if (nread == -1)
    {
        perror("read error:");
        //Close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else if (nread == 0)
    {
        fprintf(stderr, "client close.\n");
        //Close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else
    {
        printf("read message is : %s", buf);
        //修改描述符对应的事件，由读改为写
        modify_event(epollfd, fd, EPOLLOUT);
    }
}

static void do_write(int epollfd, int fd, char *buf)
{
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1)
    {
        perror("write error:");
        //Close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    }
    else
        modify_event(epollfd, fd, EPOLLIN);
    memset(buf, 0, BUFFERSIZ);
}


static void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if((epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)) == -1)
    {
        perror("epoll_ctl: add");
    }
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if((epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev)) == -1)
    {
        perror("epoll_ctl: del");
    }
    Close(fd);    
    // 如果描述符fd已关闭，再从epoll中删除fd，则会出现epoll failed: Bad file descriptor问题
    // 所以要先从epoll中删除fd，在关闭fd. 具体可参考博文http://www.cnblogs.com/scw2901/p/3907657.html
}

static void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if((epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev)) == -1)
    {
        perror("epoll_ctl: mod");
    }
}









