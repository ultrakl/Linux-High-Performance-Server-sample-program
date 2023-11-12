//10-1 统一事件源


#include <cstring>
// #include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
// #include <string>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/fcntl.h>
// #define __GNUC__SOURCE 
#include <signal.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];


int setfdnonblock(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}


void addfd(int epollfd, int fd){
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    event.events |= EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setfdnonblock( fd );
    return;
}

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    //sa.sa_flags |= SA_RESTART;
    sigemptyset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char* argv[]) {

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr* )&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5);
    assert(epoll_fd != -1);
    addfd(epoll_fd, listenfd);

    /*使用socketpair创建管道*/
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setfdnonblock(pipefd[1]);
    addfd(epoll_fd, pipefd[0]);

    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);
    bool stop_server = false;

    while(!stop_server) {
        int number = epoll_wait(epoll_fd, events
        , MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno == EINTR)
        {
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
                addfd(epoll_fd, connfd);
            }
            else if(sockfd == pipefd[0] && events[i].events && EPOLLIN) {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals)-1, 0);
                if(ret == -1)continue;
                else if(ret == 0)continue;
                else {
                    for (int i = 0; i < ret; ++i) {
                        switch (signals[i]) {
                            case SIGCHLD:
                            case SIGHUP: break;
                            case SIGTERM: 
                            case SIGINT : {
                                printf("SIGINT\n");
                                stop_server = true;}
                        }
                        printf("%d\n", signals[i]);
                    }
                }
            }
            
        }
    }
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    return 0;
}

