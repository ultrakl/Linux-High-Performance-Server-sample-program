//9-4 使用epolloneshot事件

#include<unistd.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<pthread.h>


#define MAX_EVENT_NUMBER 1024
#define BUFF_SIZE 10

struct fds{
    int epollfd;
    int sockfd;
};

int setfdnonblock(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, bool enableET, int fd){
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    if( enableET ){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setfdnonblock( fd );
    return;
}

void reset_oneshot(int epollfd, int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void* worker(void * arg){
    int sockfd = ((fds*)arg) ->sockfd;
    int epollfd = ((fds*)arg) ->epollfd;
    printf("start thread %ld receiving data on fd %d\n", pthread_self(), sockfd);
    char buf[BUFF_SIZE];
    memset(buf, '\0', BUFF_SIZE - 1);
    while(1){
        int ret = recv(sockfd, buf, BUFF_SIZE-1, 0);
        if(ret == 0){
            close(sockfd);
            break;
        }
        else if(ret < 0){
            if(errno == EAGAIN){
                reset_oneshot(epollfd, sockfd);
                printf("read later\n");
                break;
            }
            close(sockfd);
            break;
        }
        else{
            printf("get content: %s\n", buf);
            sleep(1);
        }
    }
    // printf("end thread %ld  receving data on fd %d\n", pthread_self(), sockfd);
    //printf("end thread receving data on fd %d\n", sockfd);
    //pthread_exit(arg);
    return NULL; 
}

int main(int argc, char* argv[])
{
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);  

    ret = listen(listenfd, 5);
    assert(ret != -1);  

    int epollfd = epoll_create(5);
    /* epoll_event event1;
    event1.data.fd = listenfd;
    event1.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event1); */

    /*
    在这里为监听socket设置EPOLL_ET是不合理的，若多个socket同时连接，那可能会出现主线程处理连接不及时而导致监听队列积攒socket
    下次EPOLL_IN事件可能无法触发
    */
    addfd(epollfd, false, listenfd);
    epoll_event event[MAX_EVENT_NUMBER];
    
    while(1){
        ret = epoll_wait(epollfd, event, MAX_EVENT_NUMBER, -1);
        sleep(6);
        if(ret < 0){
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < ret; i++){
            int sockfd = event[i].data.fd;
            if(sockfd == listenfd){ 
                while(event[i].events & EPOLLIN){
                sockaddr_in client_address;
                socklen_t len = sizeof(client_address);
                int connfd = accept(listenfd, (sockaddr*)&client_address, &len);
                addfd(epollfd, true, connfd);
                printf("connected\n");
                }
                //reset_oneshot(epollfd, listenfd);
            }
            else if(event[i].events & EPOLLIN){
                pthread_t thread;
                fds for_new_worker;
                for_new_worker.epollfd = epollfd;
                for_new_worker.sockfd = sockfd;
                pthread_create(&thread, NULL, worker, (void*)&for_new_worker);
            }
            else {
                printf("can't handle this\n");
            }
        }
    }
    close(listenfd);
    return 0;
}

