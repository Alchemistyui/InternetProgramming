#include "unp.h"
#include "string.h"
#include <stdio.h>

//消息的结构体,
typedef struct msg {
    char type; // 消息类型,消息分为登陆、广播、退出三种，分别对应 L、B、Q
    char name[32]; // 消息来源
    char text[MAXLINE]; // 消息内容
}MSG;

//存储已连接客户端的链表 
typedef struct node {
    struct sockaddr_in addr; //客户端地址
    struct node *next; //链表结点的指针
}listnode,*linklist; //链表结点和链表


linklist linklist_creat();
void login(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void logout(linklist H,struct sockaddr_in clientaddr);



int main(int argc, char** argv) {
    int sockfd;
    char msg[MAXLINE];
    pid_t childpid;//子进程获取终端输入的内容并发送,父进程接收处理客户端的消息并发送
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htol(INADDR_ANY);

    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

    childpid = fork();

    if (childpid == 0) { //在子进程内，读取消息并发送给其他客户端
        memset(&msg,0,sizeof(msg)); //创建一个消息
        strcpy(msg.name,"server"); //消息的来源是服务器
        msg.type='B'; // 消息的类型是广播

        for(;;) {
            fgets(msg.text, MAXLINE, stdin); // 读取终端输入的内容
            msg.text[strlen(msg.text)-1]='\0'; //添加字符串结束标志
            // 因为为UDP编程，用sendto函数
            // clilen = sizeof(cliaddr);
            // struct sockaddr = SA
            clilen = sizeof(struct sockaddr);
            // ？？？to的地址
            sendto(sockfd,&msg,sizeof(msg),0,(SA*)&(servaddr),addrlen);

        }
    }
    // 父进程内
    linklist H=linklist_creat();//创建一个连接列表
    // 接受客户端消息
    // ???? msg未定义
    if(recvfrom(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(cliaddr),&addrlen)
    <= 0) {
        errlog("recvfrom error"); 
    }
    //对不同的消息类型进处理
    if (msg.type == 'L') {
        login(sockfd, H, msg, cliaddr);
    }
    else if (msg.type == 'B') {
        send_message(sockfd, H, msg, cliaddr);
    }
    else {
        logout(H, cliaddr);
    }


}


//创建链表的函数
linklist linklist_creat() {
    linklist H; //定义一个链表
    H=(linklist)malloc(sizeof(listnode)); //分配一个链表结点大小的空间作为头结点
    H->next=NULL; //下一个结点指针置为空
    return H;//返回创建的链表
}


// 客户端连接服务器
// sockfd 是套接字􏰀述符，H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
void login(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p=H->next; // 指向第一个结点
    //sprintf(msg.text,"%s 上线了",msg.name); 
    strcpy(msg.text,msg.name); //将消息的来源复制到消息内容里
    msg.text[strlen(msg.text)]='\0'; //添加字符串结束标志
    strcat(msg.text," on line");//拼接字符串
    printf("%s\n", msg.text);
    // puts(msg.text);
    // 遍历链表
    while(p!=NULL)
    {   //发送至已存在的客户端通知上线消息
        sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&(p->addr),sizeof(p->addr)); 
        // printf("send %s to port %d\n",msg.text,ntohs((p->addr).sin_port));
        p=p->next; 
    }

    p=(linklist)malloc(sizeof(listnode)); //分配新结点的空间
    p->addr=clientaddr; //新节点赋值
    //使用头插法插入
    p->next=H->next; 
    H->next=p;
    
    printf("get client port = %d.\n",ntohs((p->addr).sin_port)); 
}


// 通过广播发送消息
// sockfd 是套接字􏰀述符，H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p = H->next;// 指向第一个结点
    char s[MAXLINE]={0};
    sprintf(s,"%s 说: %s",msg.name, msg.text); 
    strcpy(msg.text,s);
    // puts(msg.text);
    printf("%s\n", msg.text);
    //遍历链表
    while(p) {
        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&clientaddr,&p->addr,sizeof(clientaddr))!=0) { //不是发信客户端时
            //发送至已存在的客户端以广播消息
            if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(p->addr),sizeof(p->addr))<0) {
            errlog("fail to sendto"); 
            } 
        }
        p=p->next; 
    }
}


void logout(linklist H,struct sockaddr_in clientaddr) {
    linklist p = H;// 指向头结点
    while (p) {
        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&clientaddr,&p->next->addr,sizeof(clientaddr))==0) { //是发信客户端时
            q=p->next; 
            p->next=q->next; 
            free(q); 
            q=NULL;
            break;
        }
        p=p->next; 
    }
}











