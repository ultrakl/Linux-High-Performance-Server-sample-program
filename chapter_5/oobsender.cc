//5-7 发送带外数据

#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

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
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);
    int ret = connect(sockfd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
    if(ret < 0){
        printf("Error is %s", strerror(errno) );
    }
    else{
        const char oob_data[] = "abc";
        const char normal_data[] = "123";
        send(sockfd, normal_data, strlen(normal_data), 0);
        sleep(1);
        send(sockfd, oob_data, strlen(oob_data), MSG_OOB);
        send(sockfd, normal_data, strlen(normal_data), 0);
    }
    close(sockfd);
    
    return 0;
}


