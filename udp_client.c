#include "unp.h"
#include <string.h>

//消息的结构体,
typedef struct msg {
    // struct sockaddr_in toaddr; //收件客户端地址
    char type; // 消息类型,消息分为登陆、广播、退出三种，分别对应 L、B、Q
    char name[32]; // 消息来源
    char to[32]; // 消息接受者
    char text[MAXLINE]; // 消息内容
}MSG;


// //存储已连接客户端的链表 
// typedef struct node {
//     struct sockaddr_in addr; //客户端地址
//     struct node *next; //链表结点的指针
// }listnode,*linklist; //链表结点和链表

// //创建链表的函数
// linklist linklist_creat() {
//     linklist H; //定义一个链表
//     H=(linklist)malloc(sizeof(listnode)); //分配一个链表结点大小的空间作为头结点
//     H->next=NULL; //下一个结点指针置为空
//     return H;//返回创建的链表
// }


int main(int argc, const char *argv[]) {
    MSG msg;
    int sockfd;
    struct sockaddr_in serveraddr;
    socklen_t addrlen = sizeof(struct sockaddr);
    // int maxfd;
    // fd_set infds;
    pid_t childpid;

    if (argc != 3){
        printf("argc != 3\n");
    }

    //创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("fail to socket\n"); 
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_port= htons(SERV_PORT); 
    // serveraddr.sin_addr.s_addr=htonl(INADDR_ANY); 
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr); 
    // while(1){
    //     FD_ZERO(&infds); 
    //     FD_SET(fileno(stdin),&infds); 
    //     FD_SET(sockfd,&infds); 
    //     maxfd=max(fileno(stdin),sockfd)+1;
    //     if (select(maxfd,&infds,NULL,NULL,NULL)==-1){
    //         printf("select io error\n"); 
    //         return -1;
    //     }
        // 发送连接的消息
        msg.type = 'L';
        // msg.name = argv[2];
        strcpy(msg.name, argv[2]);
        if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&serveraddr,addrlen)<0)
        {
            printf("fail to sendto\n");
        } 

    while(1){
        childpid = fork();

        if (childpid == 0) { //在子进程内
            // 子进程负责获取终端输入内容并发送给服务器
            char in[MAXLINE+32];
            // fgets(msg.text,MAXLINE,stdin);
            // msg.text[strlen(msg.text)-1]='\0';//添加字符串结尾

            fgets(in,MAXLINE,stdin);
            in[strlen(in)-1]='\0';//添加字符串结尾
            if(strncmp(in,"quit",4)==0)//当输入命令为退出时
            {
                msg.type='Q'; //消息类型为退出
                //发送退出消息至服务器
                if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&serveraddr,addrlen)<0) {
                    printf("fail to sendto\n"); 
                }
                // printf("quit !\n"); 
                kill(getppid(),SIGKILL); //杀死读取输入的子进程本身
                exit(1);
            } 
            else { //当不为退出为发送消息时
                // char *to = NULL;
                char to[32];
                // char delims[] = " ";

                // to = strtok(msg.text, delims);//分离出目的地别名
                // printf("%s\n", to);
                // printf("%s\n", msg.text);
                for (int i=0; i<strlen(in); i++) {
                    if(in[i]!=' ') {
                        to[i] = in[i];
                    }
                    else {
                        // msg.text = strchr(in, ' ');
                        strcpy(msg.text, strchr(in, ' ')+1);
                    }
                }

                // printf("接受者 %s\n", to);
                // printf("%s\n", msg.text);

                msg.type='B'; //消息类型设为广播
                // msg.to=to;
                strcpy(msg.to, to);
                strcpy(msg.name, argv[2]);
                //发送广播消息给服务器
                if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&serveraddr,addrlen)<0)
                {
                    printf("fail to sendto\n");
                }
            }
        }
        // 在父进程内，父进程负责接收服务器的消息并打印
        // if(memcmp(&clientaddr,&p->next->addr,sizeof(clientaddr))==0) {

        // }
        recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, &addrlen); 
        // puts(msg.text);
        printf("%s\n", msg.text);



    }
}













// int main(int argc, char **argv) {
//     int sockfd; 
// struct sockaddr_in servaddr;
//     if (argc != 2){
//         err_quit("usage: udpcli <IPaddress>");
//     }
    
//     bzero(&servaddr, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(SERV_PORT);
//     Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//     bzero(&cliaddr, sizeof(cliaddr));
//     cliaddr.sin_family = AF_INET;
//     cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//     cliaddr.sin_port = htons(0); /* force assignment of ephemeral port */ 

//     bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
//     dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));
//     exit(0); 
// }