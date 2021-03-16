/*
IO复用-epoll
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

#define MAX_EVENTS 5
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
    int epfd = epoll_create(MAX_EVENTS);
    struct epoll_event ev;
    struct epoll_event evlist[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1){
        printf("error epoll_ctl");
        return 5;
    }

    maxi = 1;

    while(1){
        
        int ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
        if(ready == -1){
            if(errno == EINTR)continue;
            else{
                printf("error epoll_wait");
                return 6;
            }
        }

        for(i = 0; i < ready; ++i){
            int sockfd = evlist[i].data.fd;
            if(sockfd == listenfd && (evlist[i].events & EPOLLIN)){
                struct sockaddr_in cliaddr;
                socklen_t clilen = sizeof(cliaddr);
                int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
                ev.events = EPOLLIN | EPOLLRDHUP;
                ev.data.fd = connfd;
                if(maxi <= MAX_EVENTS){
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
                    maxi++;
                }else{
                    printf("too many connect\n");
                }
            }
            else if(evlist[i].events & EPOLLRDHUP){
                printf("client%d closed\n", sockfd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &evlist[i]);
                maxi--;
                if(maxi <= 1){
                    printf("all connect had closed, close the server\n");
                    return 0;
                }
            }
            else if(evlist[i].events & EPOLLIN){
                
                if((n = read(sockfd, buf, sizeof(buf)-1)) < 0){
                    if(errno == ECONNRESET){
                        close(sockfd);
                    }else{
                        printf("read error");
                    }
                }else if(n == 0){
                    close(sockfd);
                }else{
                    buf[n] = '\0';
                    printf("client%d: %s", sockfd, buf);
                    fflush(stdout);
                    write(sockfd, buf, n);
                }
            }
        }
    }
    return 0;
}