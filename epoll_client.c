#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFERSIZ    1024
#define SERV_PORT   9877
#define FDSIZE        1024
#define EPOLLEVENTS 100

void handle_connection(int sockfd);
void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf);
void add_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);

ssize_t Read(int fd, void *ptr, size_t nbytes);
void Write(int fd, void *ptr, size_t nbytes);
ssize_t writen(int fd, const void *vptr, size_t n);
void Writen(int fd, void *ptr, size_t nbytes);

void Close(int fd);

int main(int argc, char **argv)
{
    if(argc != 2)
        perror("usage: tcpcli <IPaddress>");
    
    int sockfd;
    struct sockaddr_in  servaddr;
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket error\n");
        exit(1);
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if((inet_pton(AF_INET, argv[1], &servaddr.sin_addr)) <= 0)
    {
        perror("inet_pton error");
        exit(1);
    }

    if((connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) < 0)
    {
        perror("connect error\n");
        exit(1);
    }
    
    handle_connection(sockfd);
    
    Close(sockfd);
    exit(0);
}

void handle_connection(int sockfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    char buf[BUFFERSIZ];
    int num;
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd, sockfd, EPOLLIN);
    add_event(epollfd, STDIN_FILENO, EPOLLIN);
    for ( ; ; )
    {
        num = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, num, sockfd, buf);
    }
    Close(epollfd);
}

void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf)
{
    int fd;
    int i;
    int stdineof = 0;
    for (i = 0; i < num; i++)
    {
        fd = events[i].data.fd;
        if(fd == sockfd)
        {
            int nread;
            nread = Read(sockfd, buf, BUFFERSIZ);
            if (nread == 0)
            {
                if(stdineof == 1)
                    return;
                fprintf(stderr, "server close\n");
                Close(sockfd);
                Close(epollfd);
                exit(1);
            }
            Write(fileno(stdout), buf, nread);
        }
        else
        {
            int nread;
            nread = Read(fd, buf, BUFFERSIZ);
            if (nread == 0)
            {
                stdineof = 1;
                fprintf(stderr, "no inputs\n");
                Close(fd);
                continue;
            }
            modify_event(epollfd, sockfd, EPOLLOUT);
            Writen(sockfd, buf, nread);
            modify_event(epollfd, sockfd, EPOLLIN);
            memset(buf, 0, BUFFERSIZ);
        }
    }
}

void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl: add");
    }
}

void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1)
    {
        perror("epoll_ctl: del");
    }
}

void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        perror("epoll_ctl: mod");
    }
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ( (n = read(fd, ptr, nbytes)) == -1)
        perror("read error");
    return(n);
}

void Write(int fd, void *ptr, size_t nbytes)
{
    if (write(fd, ptr, nbytes) != nbytes)
        perror("write error");
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t        nleft;
    ssize_t        nwritten;
    const char    *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;        /* and call write() again */
            else
                return(-1);            /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

void Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != nbytes)
        perror("writen error");
}

void Close(int fd)
{
    if((close(fd)) < 0)
    {
        perror("close error");
        exit(1);
    }
}

cliepoll