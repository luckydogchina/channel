#include"channel.h"
#include<sys/time.h>

#undef WIN32
#include"CTLogEx/log.h"

using namespace std;
void channel::operator<<(const void *e){
    int res;
    if(is_shut_down){
        ERROR("the channel had been shut down\n");
        throw EOF;
    }

    if(capacity>0){
        int num;
        res = sem_getvalue(&this->channel_sem, &num);
        if(num >= capacity){
            ERROR("the capacity is not enough: capacity %d\n", capacity);
            throw -2;
        }
    }

    pthread_mutex_lock(&this->mutex);
    if (this->header == nullptr){
        this->header = new(element);
        this->header->data = (void*)e;

        this->end = this->header;
    }else{
        element *new_element = new(element);
        new_element->data = (void*)e;
        this->end->next = new_element;
        new_element->front = this->end;
        this->end = new_element;
    }
    pthread_mutex_unlock(&this->mutex);

    res = sem_post(&this->channel_sem);
    if (res != 0){
        ERROR("sem_post error: %d\n", res);
        throw res;
    }
    return;
}

void channel::operator>>(void **data){
    if(is_shut_down){
        *data = nullptr;
        WARN_1("the channel had been shut down\n");
        return;
    }

    //阻塞等待
    int res;
    if (timeout != 0){
        struct timeval now;
        struct timespec until;
        gettimeofday(&now, NULL);
        until.tv_sec = now.tv_sec;
        until.tv_nsec = (now.tv_usec + timeout*1000)*1000;
        res = sem_timedwait(&this->channel_sem, &until);
        if(res != 0){
            WARN_1("the time %d usces is out: %d\n", timeout, res);
            throw res;
        }
            
    }else{
        res = sem_wait(&this->channel_sem);
        if(res != 0){
            ERROR("sem_wait error: %d\n", res);
            throw res;
        }
    }

    pthread_mutex_lock(&this->mutex);
    if (this->header != nullptr){
        *data = this->header->data;
        element* e = this->header;
        this->header = e->next;
        delete(e);
        e=nullptr;
        if (this->header == nullptr){
            this->end = nullptr;
        }
    }else{
        *data = nullptr;
    }
    pthread_mutex_unlock(&this->mutex);

    return;
}

void channel::eof(int num){    
    try
    {
        for (int i =0; i < num; i++){
            this->operator<<(nullptr);    
        }
    }
    catch(const int & e)
    {
        // stderr << e.what() << '\n';
        WARN_1("catch a int type execption: %d", e);
    }    
}

void channel::shutdown(){
    this->is_shut_down = true;
    sem_post(&this->channel_sem);
}