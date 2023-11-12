//9-6 聊天室客户端

#define _GNU_SOURCE 1
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <poll.h>
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
int main(int argc, char* argv[])
{
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int connfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(connfd >= 0);

    struct sockaddr_in client_address;
    bzero(&client_address, sizeof(client_address));
    client_address.sin_port = htons(port);
    client_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &client_address.sin_addr);
    int ret = connect(connfd, (struct sockaddr* )&client_address, sizeof(client_address));
    assert(ret != -1);

    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = connfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    char readbuf[BUFFER_SIZE];
    int pipefd[2];
    ret = pipe(pipefd);
    assert(ret != -1);

    while (1) {
        ret = poll(fds, 2, -1);
        if(ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        
        if(fds[1].revents & POLLRDHUP)
        {
            printf("connection closed by peer\n");
            break;
        }
        else if (fds[1].revents & POLLIN) {
            memset(readbuf, 0, BUFFER_SIZE - 1);
            recv(connfd, readbuf, BUFFER_SIZE - 1, 0);
            printf("%s\n", readbuf);
        }
        if (fds[0].events & POLLIN) {
            ret = splice(0, NULL, pipefd[1], NULL, 32678, SPLICE_F_MORE | SPLICE_F_MOVE);
            //printf("%d bytes\n", ret);
            //tee(pipefd[0], 1, 32, SPLICE_F_NONBLOCK);
            ret = splice(pipefd[0], NULL, connfd, NULL, 32678, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
        
    }
    close(connfd);
    return 0;
}