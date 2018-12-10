#include "unp.h"
#include "string.h"
#include <stdio.h>

//感觉还需要记录收件人地址，也可采用广播然后客户端自己比对，因为发信者不好获取收信人地址
//消息的结构体,
typedef struct msg {
    // struct sockaddr_in toaddr; //收件客户端地址
    char type; // 消息类型,消息分为登陆、广播、退出三种，分别对应 L、B、Q
    char name[32]; // 消息来源
    char to[32]; // 消息接受者
    char text[MAXLINE]; // 消息内容
}MSG;

//感觉还需要记录端口
//存储已连接客户端的链表 
typedef struct node {
    struct sockaddr_in addr; //客户端地址
    char name[32]; // 客户端别名
    struct node *next; //链表结点的指针
}listnode,*linklist; //链表结点和链表


linklist linklist_creat();
void login(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void broadcast(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void logout(linklist H,struct sockaddr_in clientaddr);



int main(int argc, char** argv) {
    int sockfd;//套接字文件描述符
    // char msg[MAXLINE];//存放消息的字符数组
    MSG msg;//定义消息
    pid_t childpid;//子进程获取终端输入的内容并发送,父进程接收处理客户端的消息并发送
    struct sockaddr_in cliaddr, servaddr;
    socklen_t addrlen;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
   
    // //循环接受请求
    // for(;;) {
//自组织时才需要感觉
        // childpid = fork();//创建子进程
        // if (childpid == 0) { //在子进程内,获取终端输入的内容并发送
        //     memset(&msg,0,sizeof(msg)); //初始化消息
        //     strcpy(msg.name,"server"); //消息的来源是服务器
        //     msg.type='B'; // 消息的类型是广播

        //     for(;;) {
        //         fgets(msg.text, MAXLINE, stdin); // 读取终端输入的内容
        //         msg.text[strlen(msg.text)-1]='\0'; //添加字符串结束标志
        //         // 因为为UDP编程，用sendto函数
        //         // clilen = sizeof(cliaddr);
        //         // struct sockaddr = SA
        //         clilen = sizeof(struct sockaddr);
        //         // ？？？to的地址
        //         // sendto(sockfd,&msg,sizeof(msg),0,(SA*)&(servaddr),addrlen);
        //         sendto(sockfd,&msg,sizeof(msg),0,(SA*)&(cliaddr),addrlen);

        //     }
        // }
    

        // 父进程内，读取消息并发送给客户端
        linklist H=linklist_creat();//创建一个连接列表
    //循环接受请求
    for(;;) {
        // 接受客户端消息
        if(recvfrom(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(cliaddr),&addrlen)
        <= 0) {
            printf("recvfrom error\n"); 
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
    // //sprintf(msg.text,"%s 上线了",msg.name); 
    // strcpy(msg.text,msg.name); //将消息的来源复制到消息内容里
    // msg.text[strlen(msg.text)]='\0'; //添加字符串结束标志
    // strcat(msg.text," on line");//拼接字符串
    printf("%s is online\n", msg.name);
    // puts(msg.text);

//感觉没有必要
    // // 遍历链表
    // while(p!=NULL)
    // {   //发送至已存在的客户端通知上线消息
    //     sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&(p->addr),sizeof(p->addr)); 
    //     // printf("send %s to port %d\n",msg.text,ntohs((p->addr).sin_port));
    //     p=p->next; 
    // }

    p=(linklist)malloc(sizeof(listnode)); //分配新结点的空间
    p->addr=clientaddr; //新节点赋值
    // p->name=msg.name;
    strcpy(p->name, msg.name);
    //使用头插法插入
    p->next=H->next; 
    H->next=p;
    // printf("get client port = %d.\n",ntohs((clientaddr).sin_port)); 
    printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
    // printf("get client port = %d.\n",ntohs((p->addr).sin_port)); 
}


// 通过广播发送消息
// sockfd 是套接字􏰀述符，H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
void broadcast(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p = H->next;// 指向第一个结点
    char s[MAXLINE+37]={0};
    sprintf(s,"%s 说: %s",msg.name, msg.text); 
    // strcat()
    strcpy(msg.text,s);
    // puts(msg.text);
    printf("%s\n", msg.text);
    //遍历链表
    while(p) {
        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&clientaddr,&p->addr,sizeof(clientaddr))!=0) { //不是发信客户端时
            //发送至已存在的其他客户端以广播消息
            if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(p->addr),sizeof(p->addr))<0) {
                printf("fail to sendto\n"); 
            } 
        }
        p=p->next; 
    }
}

void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p = H->next;// 指向第一个结点
    char s[MAXLINE+37]={0};
    // char dest[32]=msg.to;
    sprintf(s,"%s 说: %s",msg.name, msg.text); 
    strcpy(msg.text,s);
    // puts(msg.text);
    printf("%s\n", msg.text);
    //遍历链表,寻找目的地
    while(p) {

        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&msg.to,&p->name,sizeof(clientaddr))==0) { //是发信客户端时
            //发送至目的地客户端
            // printf("%s & %s \n", msg.to, p->name);
            // printf("%s:%d \n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
            if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(p->addr),sizeof(p->addr))<0) {
                printf("fail to sendto\n"); 
            } 
            break;
        }
        p=p->next; 
    }
}

void logout(linklist H,struct sockaddr_in clientaddr) {
    linklist p = H;// 指向头结点
    linklist q = NULL;
    // printf("log out func\n");
    while (p) {
        // printf("%s\n", *clientaddr);
        // printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        // printf("%d\n", p->next->addr.sin_port);
        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&clientaddr,&p->next->addr,sizeof(clientaddr))==0) { //是发信客户端时
            // printf("2\n");
            // printf("%d\n", p->next->addr.sin_port);
            q=p->next; 
            p->next=q->next; 
            free(q); 
            q=NULL;

            printf("%s:%d log out\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
            break;
        }
        p=p->next; 
    }
    // printf("3\n");
}



// 客户端给出收件人地址和端口，然后服务器收到后进行比对链表，找到正确的客户端，发送
// 客户端给出收件人地址和端口，服务器采用广播，客户端收到以后比对地址和端口
// 或者取别名，存入链表，直接比对收信者别名






