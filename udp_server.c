#include "unp.h"
#include "string.h"
#include <stdio.h>

//感觉还需要记录收件人地址，也可采用广播然后客户端自己比对，因为发信者不好获取收信人地址

//消息的结构体
typedef struct msg {
    // struct sockaddr_in toaddr; //收件客户端地址
    char type; // 消息类型,消息分为登陆、发信、退出三种，分别对应 L、B、Q
    char name[32]; // 消息来源别名
    char to[32]; // 消息接受者别名
    char text[MAXLINE]; // 消息内容
}MSG;


//存储已连接客户端的链表 
typedef struct node {
    struct sockaddr_in addr; //客户端地址
    char name[32]; // 客户端别名
    struct node *next; //链表结点的指针
}listnode,*linklist; //链表结点和链表


linklist linklist_creat();
void login(linklist H,MSG msg,struct sockaddr_in clientaddr);
// void broadcast(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr);
void logout(linklist H,struct sockaddr_in clientaddr);



int main(int argc, char** argv) {
    int sockfd;//套接字文件描述符
    // char msg[MAXLINE];//存放消息的字符数组
    MSG msg;//定义消息
    pid_t childpid;//子进程获取终端输入的内容并发送,父进程接收处理客户端的消息并发送
    struct sockaddr_in cliaddr, servaddr;//客户端和服务器的地址
    socklen_t addrlen;// 地址的size，方便后面recvfrom使用

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);// 创建一个监听的socket，注意此处参数与tcp的不同

    bzero(&servaddr, sizeof(servaddr));//初始化服务器地址
    // 为服务器地址结构体赋值
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));//将监听文件描述符和服务器地址绑定起来
   
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
    

        // 读取消息并发送给客户端
        linklist H=linklist_creat();//创建一个连接链表
    //循环接受请求
    for(;;) {
        // 接受客户端消息
        if(recvfrom(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(cliaddr),&addrlen)
        <= 0) {//错误处理
            printf("recvfrom error\n"); 
        }
        printf("emm: %s\n", inet_ntoa(cliaddr.sin_addr));
        //对不同的消息类型进处理
        if (msg.type == 'L') { //当消息类型为登录时         
            login(H, msg, cliaddr);
        }
        else if (msg.type == 'B') {//当消息类型为发信时
            send_message(sockfd, H, msg, cliaddr);
        }
        else {//当消息类型为退出时
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


// 客户端连接服务器，登录处理函数
// H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
void login(linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p=H->next; // 指向第一个结点
    // //sprintf(msg.text,"%s 上线了",msg.name); 
    // strcpy(msg.text,msg.name); //将消息的来源复制到消息内容里
    // msg.text[strlen(msg.text)]='\0'; //添加字符串结束标志
    // strcat(msg.text," on line");//拼接字符串
    printf("%s is online\n", msg.name);//输出提示信息
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
    printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);//输出提示信息
    // printf("get client port = %d.\n",ntohs((p->addr).sin_port)); 
}


// // 通过广播发送消息
// // sockfd 是套接字􏰀述符，H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
// void broadcast(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
//     linklist p = H->next;// 指向第一个结点
//     char s[MAXLINE+37]={0};
//     sprintf(s,"%s 说: %s",msg.name, msg.text); 
//     // strcat()
//     strcpy(msg.text,s);
//     // puts(msg.text);
//     printf("%s\n", msg.text);
//     //遍历链表
//     while(p) {
//         // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
//         if(memcmp(&clientaddr,&p->addr,sizeof(clientaddr))!=0) { //不是发信客户端时
//             //发送至已存在的其他客户端以广播消息
//             if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(p->addr),sizeof(p->addr))<0) {
//                 printf("fail to sendto\n"); 
//             } 
//         }
//         p=p->next; 
//     }
// }


// 向客户端发送消息
// sockfd 是套接字􏰀述符，H 存储网络信息的链表头节点，msg 是传输的消息，clientaddr 是客户端地址
void send_message(int sockfd,linklist H,MSG msg,struct sockaddr_in clientaddr) {
    linklist p = H->next;// 指向第一个结点
    char s[MAXLINE+37]={0};//用于字符串拼接的临时字符串数组
    // char dest[32]=msg.to;
    printf("%s 对 %s 说: %s\n", msg.name, msg.to, msg.text);//输出提示信息
    sprintf(s,"%s 说: %s",msg.name, msg.text); //字符串拼接
    strcpy(msg.text,s);//拷贝字符串
    // puts(msg.text);
    //遍历链表,寻找消息接受者
    while(p) {

        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与消息目标地址是否相同
        if(memcmp(&msg.to,&p->name,sizeof(clientaddr))==0) { //是发信客户端时
            //发送至目的地客户端
            // printf("%s & %s \n", msg.to, p->name);
            // printf("%s:%d \n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
            if(sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&(p->addr),sizeof(p->addr))<0) {
                // 错误处理
                printf("fail to sendto\n"); 
            } 
            break;
        }
        p=p->next; 
    }
}

// 退出的处理函数
// H 存储网络信息的链表头节点，clientaddr 是客户端地址
void logout(linklist H,struct sockaddr_in clientaddr) {
    linklist p = H;// 指向头结点
    linklist q = NULL;//用于删除节点的临时变量
    // printf("log out func\n");
    // 遍历链表寻找要删除的结点
    while (p) {
        // printf("%s\n", *clientaddr);
        // printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        // printf("%d\n", p->next->addr.sin_port);
        // memcmp是比较内存区域buf1和buf2的前count个字节,此处用于判断结点存储的地址与要删除节点的地址是否相同
        if(memcmp(&clientaddr,&p->next->addr,sizeof(clientaddr))==0) { //是要删除节点时
            // printf("2\n");
            // printf("%d\n", p->next->addr.sin_port);
            // 删除节点
            q=p->next; 
            p->next=q->next; 
            free(q); 
            q=NULL;

            printf("%s:%d log out\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);//输出提示信息
            break;
        }
        p=p->next; 
    }
    // printf("3\n");
}



// 客户端给出收件人地址和端口，然后服务器收到后进行比对链表，找到正确的客户端，发送
// 客户端给出收件人地址和端口，服务器采用广播，客户端收到以后比对地址和端口
// 或者取别名，存入链表，直接比对收信者别名



// 本实验的设计采用的是自组织模式。首先，服务器和客户端分别定义了相同的消息结构体用于UDP的信息传输，
// 在服务器端还定义了链表，用于保存在线的客户端的信息。在信息交互过程中，客户端先创建一个套接字，
// 用于连接服务器，然后为客户端地址结构体赋值后，由于不需要connect，直接发送一个登陆的信息。
// 在服务器端，先创建一个套接字用于交互。然后为自己的地址结构体赋值，将套接字绑定至地址后，创建一个连接链表用于维持在线客户端信息，
// 然后进入循环接收客户端消息并进行响应的状态。若服务器接收到的是登录消息，则创建一个新的链表结点插入链表，并将此客户端的信息保存至结点中；
// 若读取到的是发信消息，则服务器作为中介，读取发送客户端的消息和接受者别名，在链表中查找到该别名所对应的真正地址，并将此消息转发至接受者；
// 若读取到的是退出消息，则服务器将链表中的此客户端的信息结点删除。
// 而客户端接收到消息后，直接读取消息并打印出来即可；当用户要发送消息时，客户端从终端读取用户输入，若消息为退出命令，则向服务器发送退出消息，
// 若为发信信息时，客户端读取分离出接受者别名和消息内容，并赋值封装好后发送至服务器。


