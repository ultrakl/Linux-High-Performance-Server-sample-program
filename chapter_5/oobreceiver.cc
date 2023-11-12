//5-7 接收带外数据


#include <cstring>
#include <iostream>
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

const int buff_size = 1024;
static bool stop = false;
static void handle_term( int sig ){
    stop = true;
}

int main(int argc, char* argv[])
{
    signal( SIGTERM, handle_term);
    if(argc <= 2){
        printf("usage: %s ip_address port_number backlog\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    //int backlog = atoi(argv[3]);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int ret = bind(sock, (struct sockaddr* )&address, sizeof(address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);
    //sleep(20);
    struct sockaddr_in client;
    socklen_t client_length = sizeof(client);
    int connfd = accept(sock, reinterpret_cast<struct sockaddr*>(&client), &client_length); 
    if(connfd < 0){
        std::cout<< strerror(errno) <<std::endl;
        STDOUT_FILENO;
    }
    else{
        char remote[INET_ADDRSTRLEN];
        // std::cout<< inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN)<<" "<< ntohs(client.sin_port);
        char buf[buff_size];
        memset(buf, '\0', sizeof(buf));
        ret = recv(connfd, buf, buff_size-1, 0);
        printf("got %d bytes of normal data %s from %s \n", ret, buf, inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN));

        memset(buf, '\0', sizeof(buf));
        ret = recv(connfd, buf, buff_size-1, MSG_OOB);
        printf("got %d bytes of oob data %s from %s \n", ret, buf, inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN));

        memset(buf, '\0', sizeof(buf));
        ret = recv(connfd, buf, buff_size-1, 0);
        printf("got %d bytes of %s normal data from %s \n", ret, buf, inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN));

        close(connfd);
        
    }
    // socklen_t len = sizeof(client);
    // getsockname(connfd, reinterpret_cast<sockaddr*>(&client), &len);
    // getsockopt(int fd, int level, int optname, void *__restrict optval, socklen_t *__restrict optlen);
    // int val = 1;
    // setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
     while(!stop){
        sleep(1);
    }
    // struct linger so_linger{1, 3};
    // setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    //非线程安全，不可重入：
    auto i = gethostbyname("www.google.com");
    auto j = getservbyname("daytime", "tcp");
    //线程安全，可重入re_entrant
    // auto i1 = gethostbyname_r(const char *__restrict name, struct hostent *__restrict result_buf, char *__restrict buf, size_t buflen, struct hostent **__restrict result, int *__restrict h_errnop)
    close(sock);
    //pipe(int *pipedes);
    size_t i2 = 0;
    char* file_buf;
    //stat(const char *__restrict file, struct stat *__restrict buf)
    //S_ISDIR(mode);
    return 0;
}