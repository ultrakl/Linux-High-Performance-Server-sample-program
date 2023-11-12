// 9-7 聊天室服务端


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
#define USER_LIMIT 5
#define FD_LIMIT 65535

struct client_data{
    sockaddr_in address;
    char* writebuf;
    char buf[BUFFER_SIZE];
};

int setfdnonblock(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
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

    client_data* user = new client_data[FD_LIMIT];
    pollfd fds[USER_LIMIT + 1];
    int user_count = 0;
    for (int i = 1; i <= USER_LIMIT; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR ;//POLLERR ignored
    fds[0].revents = 0;

    while(1) {
        ret = poll(fds, user_count + 1, -1);
        if(ret < 0){
            printf("poll failure\n");
            break;
        }
        for(int i = 0; i < user_count + 1; i++) {
            if(fds[i].fd == listenfd && fds[i].revents & POLLIN) {
                struct sockaddr_in client_address;
                socklen_t client_address_len = sizeof(client_address);
                int sockfd = accept(fds[i].fd, (struct sockaddr*)&client_address, &client_address_len);
                if(sockfd < 0) {
                    printf("error is %s\n", strerror(errno));
                    continue;
                }
                if(user_count >= USER_LIMIT) {
                    const char* info = "too many connections\n";
                    printf("%s\n", info);
                    send(sockfd, info, strlen(info), 0);
                    close(sockfd);
                    continue;
                }
                setfdnonblock(sockfd);
                fds[++user_count].fd = sockfd;
                fds[user_count].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_count].revents = 0;
                printf("accept a new connection, now have %d users\n", user_count);
            }
            else if (fds[i].revents & POLLERR) {
                printf("get an error from %d\n", fds[i].fd);
                char errors[100];
                memset(errors, 0, sizeof(errors));
                socklen_t error_length = sizeof(errors);
                ret = getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &error_length);
                if(ret < 0){
                    printf("get option failed\n");
                }
                continue;
            }
            else if (fds[i].revents & POLLRDHUP) {
                user[fds[i].fd] = user[fds[user_count].fd];
                close(fds[i].fd);
                fds[i] = fds[user_count];
                i--;
                user_count--;
                printf("a client left\n");
            }
            else if (fds[i].revents & POLLIN) {
                int sockfd = fds[i].fd;
                memset(user[sockfd].buf, '\0', BUFFER_SIZE);
                ret = recv(fds[i].fd, user[sockfd].buf, BUFFER_SIZE - 1, 0);
                if(ret < 0) {
                    if(errno != EAGAIN) {
                        close(sockfd);
                        user[fds[i].fd] = user[fds[user_count].fd];
                        fds[i] = fds[user_count];
                        i--;
                        user_count --;
                    }
                }
                else if (ret == 0) {
                    close(sockfd);
                }
                else {
                    printf("get %d bytes of client data %s from socket %d \n", ret, user[sockfd].buf, sockfd);
                    for(int j = 1; j <= user_count; j++) {
                        if(fds[j].fd == sockfd) {
                            continue;
                        }
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        user[fds[j].fd].writebuf = user[sockfd].buf;
                        
                    }   
                }
                
            }
            else if (fds[i].revents & POLLOUT) {
                int sockfd = fds[i].fd;
                if(user[sockfd].writebuf == NULL) {
                    continue;
                }
                ret = send(sockfd, user[sockfd].writebuf, strlen(user[sockfd].writebuf), 0);
                user[sockfd].writebuf = NULL;
                fds[i].events |= POLLIN;
                fds[i].events |= ~POLLOUT;
            }
        }
        
    }
    delete[] user;
    close(listenfd);
    return 0;
}