//14-3 在多线程程序中调用fork函数
#include <pthread.h>
// #include <mutex>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
pthread_mutex_t mutex;
void* another(void* arg) {
    sleep(5);
    pthread_mutex_lock(&mutex);
    printf("in child thread, lock the mutex %p\n", &mutex);
    pthread_mutex_unlock(&mutex);
    printf("parent release the lock\n");
    return nullptr;
}
void prepare() {
    pthread_mutex_lock(&mutex);
}
void infork() {
    pthread_mutex_unlock(&mutex);
}
int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_t id;
    pthread_create(&id, nullptr, another, nullptr);
    sleep(1);
    //pthread_atfork(prepare, infork, infork);
    int pid = fork();
    if(pid < 0) {
        pthread_join(id, NULL);
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    else if(pid == 0) {
        pthread_mutex_lock(&mutex);
        printf("child process get the lock %p\n", &mutex);
        sleep(10);
        pthread_mutex_unlock(&mutex);
        printf("child release the lock\n");
        pthread_mutex_destroy(&mutex);
        return 0;
    }
    else {
        printf("not block?\n");
        wait(NULL);
    }
    pthread_join(id, NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
