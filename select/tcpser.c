/*
IO复用-select
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <fcntl.h>

//可连接套接字个数
#define FD_SETMAX 1024
int client[FD_SETMAX];

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Usage: %s ip port", argv[0]);
        return 1;
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("listenfd");
        return 2;
    }
    printf("listenfdet:%d\n", listenfd);

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(argv[1]);
    local.sin_port = htons(atoi(argv[2]));

    if(bind(listenfd, (struct sockaddr*)&local, sizeof(local)) < 0){
        perror("bind");
        return 3;
    }

    if(listen(listenfd, 5) < 0){
        perror("listen");
        return 4;
    }

    int maxfd = listenfd;
    int maxi = -1;
    int i;
    for(i = 0; i < FD_SETMAX; ++i){
        client[i] = -1;
    }

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    char buf[128];

    while(1){
        rset = allset;
        int ready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset)){
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);
            int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
            if(connfd < 0){
                perror("accept");
                continue;
            }
            printf("get new link: {%s:%d}\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            fflush(stdout);
            for(i = 0; i < FD_SETMAX; ++i){
                if(client[i]<0){
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETMAX){
                printf("too many clients");
            }
            FD_SET(connfd, &allset);
            if(connfd > maxfd)
                maxfd = connfd;
            if(i > maxi)
                maxi = i;
            //没有除监听套接字之外的套接字读就绪
            if(--ready <= 0)
                continue;
        }
        
        for(i = 0; i <= maxi; ++i){
            memset(buf, '\0', 128);
            int sockfd = client[i];
            if(sockfd < 0)continue;

            if(FD_ISSET(sockfd, &rset)){
                size_t s = read(sockfd, buf, sizeof(buf)-1);
                if(s > 0){
                    buf[s] = '\0';
                    //printf("from {%s:%d} : %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);
                    printf("client%d: %s", i, buf);
                    fflush(stdout);
                    //printf("%d", strlen(buf));
                    write(sockfd, buf, strlen(buf));
                }
                else if(s == 0){
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }

                if(--ready <= 0)
                    break;
            }
        }
    }
    return 0;
}