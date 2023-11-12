// 9-7 同时处理tcp和udp请求的echo服务器 


#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1023
#define USER_LIMIT 5
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024

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



