#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <fcntl.h>

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
    memset(buf, '\0', 1024);
    int maxfd;
    fd_set rset;
    FD_ZERO(&rset);
    while(1){        
        //每次select后rset会被设置为可读套接字的集合
        //每次select前需要重新设置rset，否则不能检测描述符变化
        FD_SET(fileno(stdin), &rset);
        FD_SET(sockfd, &rset);
        
        //printf("stdin: %d\n", fileno(stdin));
        maxfd = sockfd+1;//终端启动程序，stdin的文件描述符为0
        select(maxfd, &rset, NULL, NULL, NULL);

        //printf("Enter the key:");
        //fflush(stdout);

        if(FD_ISSET(fileno(stdin), &rset)){
            ssize_t s = read(0, buf, sizeof(buf)-1);
            if(s > 0){
                buf[s] = '\0';
                write(sockfd, buf, strlen(buf));
            }
        }      
        //服务器关闭时，会发出FIN，让套接字可读，套接字读出字符数为0判断服务器关闭
        //普通版client：输入 -> write -> read
        if(FD_ISSET(sockfd, &rset)){
            if(read(sockfd, buf, sizeof(buf)-1) == 0){
                printf("server had closed.\n");
                return 1;
            }
            printf("server echo: %s",buf);
        }
          
    }
    return 0;
}