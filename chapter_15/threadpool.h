/*设计优点：实现了半同步/半反应堆模式的服务器，并且通过信号量和共享队列解除了主线程和工作线程的耦合关系，通用性更强
主线程需要负责监听和IO读写，子进程需要负责响应客户数据
实现缺点： 每个线程同时间只能处理一个客户连接，并且由于同一个客户连接可能被不同的线程处理，需要保持连接的无状态，
而且由于对共享队列频繁的加锁解锁操作，导致耗费的cpu时间变多*/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "/home/kuli/cpp/chapter_14/locker.h"
#include <cstdio>
#include <exception>
#include <list>
#include <pthread.h>
template <typename T>
class threadpool {
  public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);
    static void* worker(void* arg);
    void run();

  private:
    int m_thread_number;
    int m_max_requests;
    pthread_t* m_threads;
    std::list<T*> m_workqueue; /*请求队列*/
    locker m_queuelocker;
    sem m_queuestat; /*是否有任务需要处理*/
    bool m_stop;
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : m_thread_number(thread_number),
      m_max_requests(max_requests),
      m_stop(false),
      m_threads(NULL) {
        if(thread_number <= 0 || max_requests <= 0) {
            throw std::exception();
        }
        m_threads = new pthread_t[m_thread_number];
        if(!m_threads) {
            throw std::exception();
        }
        for(int i = 0; i < thread_number; i++) {
            printf("create the %dth thread\n", i);
            /*pthread_create只能绑定静态函数*/
            if(pthread_create(&m_threads[i], NULL, worker, this) != 0) {
                delete [] m_threads;
                throw std::exception();
            }
            if(pthread_detach(m_threads[i])) {
                delete [] m_threads;
                throw std::exception();
            }
        }
      }

template <typename T>
threadpool<T>::~threadpool() {
    delete [] m_threads;
    m_stop = true;
} 

template <typename T>
bool threadpool<T>::append(T* request) {
    m_queuelocker.lock();
    printf("get up threads!\n");
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.emplace_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void *arg) {
    threadpool* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run() {
    while (!m_stop) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request) continue;
        request->process();
        printf("process over!\n");
    }

}
#endif