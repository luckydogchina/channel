#include<pthread.h>
#include<unistd.h>
#include"channel.h"
#include<errno.h>

#undef WIN32
#include"CTLogEx/log.h"

void *test_phtread(void *ptr){
    channel_out* out =(channel_out*)ptr;
    
    for(int i =0;;i++){
        void* test;
        try{
            *out >> &test;
        }
        catch (int&e){
            ERROR("worker catch the exception %d: %s", e, strerror(errno));
            break;
        }
            
        if(test == nullptr){
            break;
        }else{
            TRACE("%s\n", test);
            delete[] (char*)test;
        }
    }

    TRACE("exit");
}


int main(int argc, char const *argv[])
{
    /* code */
    channel *c = new channel();
    channel_in *in = (channel_in*)c;
    pthread_t pid, pid1, pid2;

    pthread_create(&pid, NULL, test_phtread, c);
    pthread_create(&pid1, NULL, test_phtread, c);
    pthread_create(&pid2, NULL, test_phtread, c);
    for(int i = 0; i <= 20; i++){
        try{
            char* data = new char[32];
            sprintf(data, "this is the %d message", i);
            *in << (void*)data;
        }
        catch(int &e){
            ERROR("main catch the exception %d: %s", e, strerror(errno));
            break;
        }
    }
    
    in->eof(3);
    pthread_join(pid, nullptr);
    pthread_join(pid1, nullptr);
    pthread_join(pid2, nullptr);
    delete(c);
    return 0;
}
