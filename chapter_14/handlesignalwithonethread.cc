//14-5 用一个线程处理所有信号
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define handle_errno_en(en, msg) \
    do {errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)

static void* sig_handler(void* arg) {
    sigset_t * set = (sigset_t*)arg;
    int s, sig;
    for (; ; ) {
        s = sigwait(set, &sig);
        if(s != 0) {
            handle_errno_en(s, "sigwait");
            printf("signal handling thread got signal %d\n", sig);
        }
        printf("got a signal\n");
        //break;
    }
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
    pthread_create(&thread, nullptr, sig_handler, &set);
    pause();
    return 0;
}