//13-5 在进程间传递文件描述符

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <sys/msg.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

struct s{};

void send_fd(int fd, int fd_to_send)
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];
    // sizeof(s);
    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    cmsghdr cm;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    cm.cmsg_len = CONTROL_LEN;
    *(int *)CMSG_DATA(&cm) = fd_to_send;
    //(int *)CMSG_DATA(&cm)是右值，不能用来赋值，但解引用后的左值可以, alternative:
    // memcpy(CMSG_DATA(&cm), &fd_to_send, sizeof(int));
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    int ret = recvmsg(fd, &msg, 0);
    assert(ret > 0);

    int fd_to_read = *(int *)CMSG_DATA(&cm);
    return fd_to_read;
}
int main(){
    int pipefd[2];
    int fd_to_pass = 0;
    int ret =socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);

    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        close(pipefd[0]);
        fd_to_pass = open("../demo.txt", O_RDWR, 0666);
        send_fd(pipefd[1], (fd_to_pass > 0) ? fd_to_pass : 0);
        close(fd_to_pass);
        exit(0);
    }

    close(pipefd[1]);
    fd_to_pass = recv_fd(pipefd[0]);
    char buf[1024];
    memset(buf, '\0', 1024);
    read(fd_to_pass, buf, 1024);
    printf("i got fd %d and data %s\n", fd_to_pass, buf);
    close(fd_to_pass);
    return 0;
}