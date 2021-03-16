/*
IO复用-poll
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#define USER_LIMIT 5
#define BUF_LIMIT 128

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

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("bind");
        return 3;
    }

    if(listen(listenfd, 5) < 0){
        perror("listen");
        return 4;
    }

    int i, maxi;
    ssize_t n;
    char buf[BUF_LIMIT];

    struct pollfd client[USER_LIMIT];
    client[0].fd = listenfd;
    client[0].events = POLLIN;
    for(i = 1; i < USER_LIMIT; ++i){
        client[i].fd = -1;
    }

    maxi = 0;

    while(1){
        int ready = poll(client, maxi+1, -1);

        if(client[0].revents & POLLIN){
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
            for(i = 1; i < USER_LIMIT; ++i){
                if(client[i].fd < 0){
                    client[i].fd = connfd;
                    break;
                }
            }
            if(i == USER_LIMIT){
                printf("too many clients");
            }
            client[i].events = POLLIN | POLLRDHUP;
            if(i > maxi)maxi = i;
            if(--ready <= 0)continue;
        }
        for(i = 1; i <= maxi; i++){
            int sockfd = client[i].fd;
            if(sockfd < 0)continue;

            if(client[i].revents & POLLRDHUP){
                printf("client%d closed", i);
                if(--ready <= 0)break;
            }

            if(client[i].revents & (POLLIN | POLLERR)){
                if((n = read(sockfd, buf, sizeof(buf)-1)) < 0){
                    if(errno == ECONNRESET){
                        close(sockfd);
                        client[i].fd = -1;
                    }else{
                        printf("read error");
                    }
                }else if(n == 0){
                    close(sockfd);
                    client[i].fd = -1;
                }else{
                    buf[n] = '\0';
                    printf("client%d: %s", i, buf);
                    fflush(stdout);
                    write(sockfd, buf, n);
                }
                if(--ready <= 0)break;
            }
        }
    }
    return 0;
}