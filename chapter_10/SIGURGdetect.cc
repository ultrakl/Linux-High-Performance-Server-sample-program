//10-3 用sigurg检测带外数据是否到达

//love that

#include <sys/syslog.h>
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
#include<signal.h>
#include<pthread.h>

#define BUFFER_SIZE 1024
static int connfd;
void sig_urg(int sig)
{
    int save_errno = errno;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    int ret = recv(connfd, buffer, BUFFER_SIZE-1, MSG_OOB);
    printf("oobdata%s\n", buffer);
    errno = save_errno;
}
void addsig(int sig, void (*sig_handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
int main(int argc, char* argv[])
{
    printf("%d, %d\n", connfd, STDIN_FILENO);
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

    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof( client_address );
    connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);

    addsig(SIGURG, sig_urg);
    fcntl(connfd, F_SETOWN, getpid());

    char buffer[BUFFER_SIZE];
    while(1)
    {
        memset(buffer, '\0',BUFFER_SIZE);
        ret = recv(connfd, buffer, BUFFER_SIZE - 1, 0);
        if(ret <= 0)break;
        printf("normal data %s\n", buffer);
    }
    close(connfd);
    close(listenfd);
    return 0;
}

