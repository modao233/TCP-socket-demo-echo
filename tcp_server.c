/*
多进程版本
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "io_base.h"

//信号处理函数，避免僵尸进程
void sig_chld(int signo){
	pid_t pid;
	int stat;

	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

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

    signal(SIGCHLD, sig_chld);

    while(1){
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int connfd = accept(listenfd, (struct sockaddr*)&client, &len);
        if(connfd < 0){
            perror("accept");
            continue;
        }

        printf("get new link: {%s:%d}\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        fflush(stdout);

        pid_t id = fork();
        if(id == 0){
            close(listenfd);
            char buf[1024];
            while(1){
                size_t s = read(connfd, buf, sizeof(buf)-1);
                if(s > 0){
                    buf[s] = '\0';
                    printf("from {%s:%d} : %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);
                    fflush(stdout);
                    write(connfd, buf, strlen(buf));
                }
                //连接关闭
                else if(s == 0){
                    printf("{%s:%d} quit\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    break;
                }
                else{
                    perror("read");
                    break;
                }
            }
            close(connfd);
            exit(0);
        }
        else{
            close(connfd);
        }
    }
    return 0;
}