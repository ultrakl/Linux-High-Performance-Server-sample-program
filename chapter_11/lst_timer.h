// #pragma once
//11-2 升序定时器链表


#ifndef LST_TIMER
#define LST_TIMER
#include <time.h>
#define BUFFER_SIZE 64
class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

class util_timer{
public:
    util_timer () : prev(NULL), next(NULL) {}
    time_t expire;
    void (*cb_func)(client_data*); /*任务回调函数*/
    client_data* user_data;
    util_timer* prev;
    util_timer* next;
};

class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL), tail(NULL) {}
    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while(tmp) {
            head = tmp -> next;
            delete tmp;
            tmp = head;       
        }
    }
    void add_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head = tail = timer;
            return;
        }
        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }
    /**
     * @brief 当某个定时任务发生变化时，调整对应的定时器在链表中的位置，该函数只考虑
     被调整的定时器的超时时间延长的情况，即该定时器需要往链表的尾部移动
     * 
     * @param timer 
     */
    void adjust_timer(util_timer* timer)
    {
        if(!timer)return;
        util_timer* tmp = timer->next;
        if(!tmp || (timer->expire < tmp->expire))
        {
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        else {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);

        }
    }
    void del_timer(util_timer* timer)
    {
        if(!timer)return;
        if(timer == head && timer == tail)
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        if(timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }
    /**
        * @brief SIGALRM信号每次被触发就在其信号处理函数（如果使用统一事件源
        则是主函数）中执行一次tick函数，以处理链表上到期的任务
        * 
        */
    void tick()
    {
        if(!head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);
        util_timer* tmp = head;
        while(tmp)
        {
            if(cur < tmp->expire)break;
            /*调用定时器的回调函数，以执行定时任务*/
            tmp->cb_func(tmp->user_data);
            head = tmp -> next;
            if(head)head->prev = NULL;
            delete tmp;
            tmp = head;
        }
    }
private:
    /*一个重载的辅助函数，它被公有的add_timer函数核adjust_timer函数调用，该函数表示
    将目标定时器timer添加到节点lst_head之后的部分链表中*/
    void add_timer(util_timer* timer, util_timer* lst_head)
    {
        util_timer* prev = lst_head;
        util_timer* tmp = prev->next;
        while(tmp)
        {
            if(timer->expire < tmp->expire){
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        if(!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }

    }

private:
    util_timer* head;
    util_timer* tail;
};
#endif