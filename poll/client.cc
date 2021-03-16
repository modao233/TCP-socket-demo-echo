/*
IO复用-poll
 */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

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
    int maxfd = 2;
    struct pollfd fd[2];
    fd[0].fd = fileno(stdin);
    fd[0].events = POLLIN;
    fd[1].fd = sockfd;
    fd[1].events = POLLIN | POLLRDHUP;

    while(1){        
        int ready = poll(fd, maxfd, -1);

        if(fd[1].revents & POLLRDHUP){
            printf("server had closed.\n");
            break;
        }

        if(fd[1].revents & POLLIN){
            memset(buf, '\0', BUF_LIMIT);
            ssize_t s = read(sockfd, buf, sizeof(buf)-1);
            buf[s] = '\0';
            printf("server echo: %s",buf);
        }

        if(fd[0].revents & POLLIN){
            memset(buf, '\0', BUF_LIMIT);
            ssize_t s = read(fileno(stdin), buf, sizeof(buf)-1);
            if(s > 0){
                buf[s] = '\0';
                write(sockfd, buf, strlen(buf));
            }
        }
    }
    close(sockfd);
    return 0;
}