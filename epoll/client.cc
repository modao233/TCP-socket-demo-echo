/*
IO复用-epoll
 */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>

#define MAX_EVENTS 5
#define BUF_LIMIT 128

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

    char buf[BUF_LIMIT];
    memset(buf, '\0', BUF_LIMIT);
    struct epoll_event ev;
    struct epoll_event evlist[2];
    int epfd = epoll_create(MAX_EVENTS);
    ev.events = EPOLLIN;
    ev.data.fd = fileno(stdin);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fileno(stdin), &ev);

    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while(1){        
        int ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);

        if(ready == -1){
            if(errno == EINTR)continue;
            else{
                printf("error epoll_wait");
                return 4;
            }
        }

        for(int i = 0; i < ready; ++i){
            if(evlist[i].data.fd == fileno(stdin)){
                memset(buf, '\0', BUF_LIMIT);
                ssize_t s = read(fileno(stdin), buf, sizeof(buf)-1);
                if(s > 0){
                    buf[s] = '\0';
                    write(sockfd, buf, strlen(buf));
                }
            }
            else if(evlist[i].events & EPOLLRDHUP){
                printf("server had closed.\n");
                return 0;
            }
            else if(evlist[i].events & EPOLLIN){
                memset(buf, '\0', BUF_LIMIT);
                ssize_t s = read(sockfd, buf, sizeof(buf)-1);
                buf[s] = '\0';
                printf("server echo: %s",buf);
            }
        }
    }
    close(sockfd);
    return 0;
}