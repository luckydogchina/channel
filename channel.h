#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>

struct element{
    element(){
        this->data = nullptr;
        this->front = nullptr;
        this->next = nullptr;
    };
    ~element(){

    }

    void* data;
    element* front;
    element* next;
};

class channel_out{
public:
    virtual void shutdown() = 0; 
    virtual void operator >> (void **data) = 0;

};

class channel_in{
public:
    virtual void shutdown() = 0; 
    virtual void operator << (const void *data) = 0;
    virtual void eof(int num) = 0;
};

class channel:channel_out, channel_in{
public:
    /*
        capacity: channel 内的容量上限 <=0 表示没有上限
        如果超出容量上限仍然会抛出异常
        timeout 是毫秒级别的，如果设置为<=0则会一直等待
        如果>0在超时后仍然未接收到数据会抛出异常 
    */
    channel(int capacity=0, int timeout=0):capacity(capacity),timeout(timeout){
        this->header = nullptr;
        this->end = nullptr;
        /*
        第一个参数是指向一个已经分配的sem_t变量。
        第二个参数pshared表示该信号量是否由于进程间通步，当pshared = 0，
        那么表示该信号量只能用于进程内部的线程间的同步。
        当pshared != 0，表示该信号量存放在共享内存区中，
        使使用它的进程能够访问该共享内存区进行进程同步。
        第三个参数value表示信号量的初始值
        */
        sem_init(&this->channel_sem, 0, 0);
        this->is_shut_down = false;
        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_TIMED_NP);
        pthread_mutex_init(&this->mutex, &mutex_attr);
    };

    ~channel(){
        element *point = this->header;
        while(1){
            if (point != nullptr){
                element* e = point;
                point = e->next;
                delete(e);
                e = nullptr; 
            }
        }
        sem_destroy(&this->channel_sem);
        pthread_mutex_destroy(&this->mutex);
    };
    
    // 在调用shutdown后如果继续
    // 向channel内写入数据会抛出异常
    // 而从channel中会持续读出空数据
    void shutdown();
    
    //此函数会在流中插入空值
    //消费者在读取到空值时会判断为传输结束
    void eof(int num);
    
    //向channel内传入数据
    void operator << (const void *data);
    
    //从chanel中读取数据
    void operator >> (void **data);
private:
    element* header;
    element* end;

    //暂时使用无名信号量
    sem_t channel_sem;
    const int timeout;
    volatile bool is_shut_down;
    const int capacity;
    pthread_mutex_t mutex;
};



