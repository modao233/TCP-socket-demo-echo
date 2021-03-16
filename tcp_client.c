#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "io_base.h"

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Usage: %s server_ip server_port", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket");
        return 2;
    }
    printf("socket:%d\n", sockfd);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("connect");
        return 3;
    }

    char buf[1024];
    while(1){
        printf("Enter the key:");
        fflush(stdout);
        //从标准输入读
        //默认从终端读取字符，遇到换行，read调用就会结束
        //Linux: Ctrl-D可停止输入，不输入换行
        ssize_t s = read(0, buf, sizeof(buf)-1);
        if(s > 0){
            buf[s] = '\0';
            write(sockfd, buf, strlen(buf));
            if(read(sockfd, buf, sizeof(buf)-1) == 0){
                printf("server had closed.\n");
                return 1;
            }
            printf("server echo: %s\n",buf);
        }
    }
    return 0;
}