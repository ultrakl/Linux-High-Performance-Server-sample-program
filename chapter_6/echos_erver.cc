//6-4 回射服务器

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
#include<pthread.h>


int main( int argc, char* argv[])
{

    int inputfd = open(argv[1], O_RDONLY);
    int filefd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    assert( filefd > 0 );

    int pipefd_stdout[2];
    int ret = pipe( pipefd_stdout );
    assert( ret != -1);

    int pipefd_file[2];
    ret = pipe( pipefd_file );
    assert( ret != -1);

    //fcntl(pipefd_file[1], F_SETFL, O_NONBLOCK);
    //fcntl(0, F_SETFL, O_NONBLOCK);
    ret = splice(0, NULL, pipefd_stdout[1], NULL, 32, SPLICE_F_MORE | SPLICE_F_MOVE | SPLICE_F_NONBLOCK
    );
    if(ret == -1){
        printf("%s", strerror(errno));
        return 0;
    }
    // assert( ret != -1 );

    ret = tee( pipefd_stdout[0], pipefd_file[1], 32, SPLICE_F_NONBLOCK );
    assert( ret != -1 );
    
    ret = splice( pipefd_file[0], NULL, filefd, NULL, 32, SPLICE_F_MORE | SPLICE_F_MOVE );
    assert( ret != -1 );

    ret = splice( pipefd_stdout[0], NULL, STDOUT_FILENO, NULL, 32, SPLICE_F_MORE | SPLICE_F_MOVE );
    printf("%d\n", ret);
    assert( ret != -1);

    close( filefd );
    close( pipefd_stdout[0] );
    close( pipefd_file[1] );
    close( pipefd_file[0] );
    close( pipefd_stdout[1] );
    // syslog(int pri, const char *fmt, ...);
    setlogmask(LOG_ALERT);
    // getaddrinfo(const char *__restrict name, const char *__restrict service, const struct addrinfo *__restrict req, struct addrinfo **__restrict pai)
    getuid();
    return 0;

} 