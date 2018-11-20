//server.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define SERV_PORT 9877
#define LISTENQ 10

// void error_handing(char* buf);

// struct sockaddr
//  {
//    __SOCKADDR_COMMON (sa_);   
//    char sa_data [14];          
//  };

int main(int argc, char* argv[])
{
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    // struct timeval timeout;
    // fd_set reads, cpy_reads;
    fd_set rset, allset;

    // socklen_t adr_sz;
    socklen_t clilen;

    int maxfd, str_len, nready, i;
    char buf[BUF_SIZE];

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    bind(listenfd,(struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); //将服务端套接字注册入fd_set,即添加了服务器端套接字为监视对象
    maxfd = listenfd;

    while(1)
    {
        rset = allset;
        // timeout.tv_sec = 5;
        // timeout.tv_usec = 5000;

        //无限循环调用select 监视可读事件
        // if((fd_num = select(maxfd+1, &rset, 0, 0, &timeout)) == -1)
        if((nready = select(maxfd+1, &rset, NULL, NULL, NULL)) == -1)
        {
            perror("select error");
            break;
        }
        if(nready == 0)
            continue;

        for(i = 0; i < maxfd + 1; i++)
        {
            if(FD_ISSET(i, &rset))
            {
                /*发生状态变化时,首先验证服务器端套接字中是否有变化.
                ①若是服务端套接字变化，接受连接请求。
                ②若是新客户端连接，注册与客户端连接的套接字文件描述符.
                */
                if(i == listenfd)
                {
                    clilen = sizeof(cliaddr);
                    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
                    FD_SET(connfd, &allset);
                    if(maxfd < connfd)
                        maxfd = connfd;
                    printf("connected client: %d \n", connfd);
                }
                else    
                {
                    str_len = read(i, buf, BUF_SIZE);
                    if(str_len == 0)    //读取数据完毕关闭套接字
                    {
                        FD_CLR(i, &allset);//从reads中删除相关信息
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else
                    {
                        write(i, buf, str_len);//执行回声服务  即echo
                    }
                }
            }
        }
    }
    close(listenfd);
    return 0;
}


// void error_handing(char* buf)
// {
//     fputs(buf, stderr);
//     fputc('\n', stderr);
//     exit(1);
// }