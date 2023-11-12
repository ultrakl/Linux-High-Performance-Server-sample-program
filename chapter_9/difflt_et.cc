//9-3 LT & ET pattern


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
#define __GNUC__SOURCE 
#define MAX_EVENT_NUMBER 1024
#define BUFF_SIZE 10

int setfdnonblock(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}


void addfd(int epollfd, bool enableET, int fd){
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if( enableET ){
        event.events |= EPOLLET;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setfdnonblock( fd );
    return;
}

void lt(epoll_event* events, int number, int epollfd, int listenfd){
    char buf[BUFF_SIZE];
    for(int i = 0; i < number; i++){
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd){
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
            addfd(epollfd, false, connfd);
        }
        else if(events[i].events & EPOLLIN){
            printf("event trigger once\n");
            memset(buf, '\0', BUFF_SIZE);
            int ret = recv(sockfd, buf, BUFF_SIZE-1, 0);
            if(ret <= 0){
                close(sockfd);
                continue;
            }
            printf("got %d bytes of content\n", ret);
        }
        else{
            printf("can't handle this\n");
        }
    }
}
void et( epoll_event* events, int number, int epollfd, int listenfd ){
    char buf[BUFF_SIZE];
    for( int i = 0; i < number; i++ ){
        int sockfd = events[i].data.fd;
        if( sockfd == listenfd ){
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof( client_address );
            int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
            addfd(epollfd, true, connfd);
        }
        else if(events[i].events & EPOLLIN){
            printf("event trigger once\n");
            memset(buf, '\0', BUFF_SIZE);
            while(1){
                int ret = recv(sockfd, buf, BUFF_SIZE-1, 0);
                if(ret < 0){
                    if(errno == EAGAIN || errno ==  EWOULDBLOCK ){
                        printf( "read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0){
                    close(sockfd);
                    break;
                }
                else{
                    printf("got %d bytes of content\n", ret);
                }
            }
        }
        else{
            printf("can't handle this\n");
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 0;
    }
    
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
    int epollfd = epoll_create(5);
    assert(epollfd >= 0);
    addfd(epollfd, true, listenfd);
    
    while(1){
        ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0){
            printf("epoll failure");
            break;
        }

        //lt(events, ret, epollfd, listenfd);
        et(events, ret, epollfd, listenfd);
    }

    close(listenfd);
    return 0;
}