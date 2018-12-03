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
#define BUFFERSIZ    1024
#define LISTENQ     1024
#define FDSIZE      1024
#define EPOLLEVENTS 100


//添加事件
static void add_event(int epollfd, int fd, int state);
//修改事件
static void modify_event(int epollfd, int fd, int state);
//删除事件
static void delete_event(int epollfd, int fd, int state);


int main(int argc, char *argv[])
{
    int  i, fd, clifd, listenfd, epollfd;
    int nread, nwrite, num;
    struct sockaddr_in servaddr, cliaddr;
    struct epoll_event events[EPOLLEVENTS];
    char buf[BUFFERSIZ];
    socklen_t  cliaddrlen;


    memset(buf, 0, BUFFERSIZ);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (listenfd == -1)
    // {
    //     perror("socket error:");
    //     exit(1);
    // }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    // if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    // {
    //     perror("bind error: ");
    //     exit(1);
    // }


    listen(listenfd, LISTENQ);


    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    add_event(epollfd, listenfd, EPOLLIN);
    for ( ; ; )
    {
        //获取已经准备好的描述符事件
        num = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        // if ((num = epoll_wait(epollfd, events, EPOLLEVENTS, -1)) == -1) 
        // {
        //     perror("epoll_pwait");
        //     exit(1);
        // }
        // handle_events(epollfd, events, num, listenfd, buf);
        // 遍历
        for (i = 0; i < num; i++)
        {
            fd = events[i].data.fd;
            //根据描述符的类型和事件类型进行处理
            if ((fd == listenfd) && (events[i].events & EPOLLIN)) {
                clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
                // if (clifd == -1)
                //     perror("accpet error:");
                // else
                // {
                    printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
                    //添加一个客户描述符和事件
                    add_event(epollfd, clifd, EPOLLIN);
                // }
            }
            else if (events[i].events & EPOLLIN) {
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
            else if (events[i].events & EPOLLOUT) {
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
        }
    }
    close(epollfd);
    exit(0);
}








static void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    close(fd);    
    // 如果描述符fd已关闭，再从epoll中删除fd，则会出现epoll failed: Bad file descriptor问题
    // 所以要先从epoll中删除fd，在关闭fd. 具体可参考博文http://www.cnblogs.com/scw2901/p/3907657.html
}

static void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
