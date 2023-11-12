//9-5 非阻塞connect

#include <future>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 1023

class top{
    using namefuck = int;
    using value = std::shared_future<int>;
};

int setnonblock(int sockfd){
    int oldopt = fcntl(sockfd, F_GETFL);
    int newopt = oldopt | O_NONBLOCK;
    fcntl(sockfd, F_SETFL, newopt);
    return oldopt;
}

int unblock_connect(const char* ip, int port, int sec){
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int fdopt = setnonblock(sockfd);
    ret = connect(sockfd, (struct sockaddr* )&address, sizeof(address));
    if(ret == 0)
    {
        printf("connect successfully\n");
        fcntl(sockfd, F_SETFL, fdopt);
        return sockfd;
    }
    else if(errno != EINPROGRESS){
        printf("unblock connect not support\n");
        return -1;
    }
    fd_set readfds;
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    timeout.tv_sec = sec;
    timeout.tv_usec = 0;

    ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
    if(ret <= 0 ){
        printf("select timeout\n");
        close(sockfd);
        return -1;
    }
    if(! FD_ISSET(sockfd, &writefds)){
        printf("no events on sockfd found\n");
    }
    int error = 0;
    socklen_t len = sizeof(error);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0){
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }
    if(error != 0){
        printf("connection failed\n");
        printf("%s\n", strerror(error));
        /*The SO_ERROR option returns and resets the per socket–based error code, getsockopt doesnot clear the errno, when the connection fails, the reason is hidden in the
          socket itself */
        printf("%s\n", strerror(errno)); 
        close(sockfd);
        return -1;
    }
    printf("connection ready after select with the socket\n");
    /* char buf[] = "fuck";
    send(sockfd, buf, 3, 0);
    read(sockfd, buf, 3);
    printf("%s\n", buf); */
    fcntl(sockfd, F_SETFL, fdopt);
    return sockfd;
}

int main(int argc, char* argv[])
{
    if(argc <=2 )
        try {
            throw"fuck";
        } catch (const char msg[]) {
            printf("%s", msg);
            return 1;
        }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = unblock_connect(ip, port, 20);
    if(sockfd < 0){
        return 0;
    }
    // sigaction(SIGTERM, )
    // sizeof(top);
    close(sockfd);
    return 0;
}