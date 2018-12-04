#include "unp.h"
#include<unistd.h> 



int main(int argc, char **argv){
    int childpid;

    if (argc != 3){
        err_quit("argc != 3");
    }

    for(int i = 0; i < atoi(argv[2]); i++) {
        // int c = atoi(argv[2]);
        // printf("c\n");
        childpid = fork();
        if (childpid < 0) // pid都是大于等于0的
        { 
            /* error occurred */
            fprintf(stderr,"Fork Failed!");
            exit(-1);
        }
        if(childpid == 0) {
            // printf("c\n");
            const char *v={argv[1]};
            execlp("/home/alchemist/mp_single_client", v, NULL);// 在子进程中加载指定的可执行文件
            // printf("emm\n");
        }
    }
    return 0;
}

// gedit multiplexing_client.c
// gcc -o multiplexing_client multiplexing_client.c -lunp
// ./multiprocess_client 127.0.0.1 3