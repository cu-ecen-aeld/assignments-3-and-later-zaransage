#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application

#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data *thread_func_args = (struct thread_data *) thread_param;

    int s;

    s = usleep(thread_func_args->wait_to_obtain_ms * 1000);
    if (s !=0){
        thread_func_args->thread_complete_success = false;
    }

    s = pthread_mutex_lock(thread_func_args->mutex);
    if (s != 0) {
        thread_func_args->thread_complete_success = false;
        return false;
    }

    s = usleep(thread_func_args->wait_to_release_ms * 1000);
    if (s !=0){
        thread_func_args->thread_complete_success = false;
    }

    s = pthread_mutex_unlock(thread_func_args->mutex);
    if (s != 0){
        thread_func_args->thread_complete_success = false;
    } 
        
    thread_func_args->thread_complete_success = true; // a thank you to https://github.com/cu-ecen-aeld/assignments-3-and-later-Suhas-Reddy-S/tree/master

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    int s;

    // I will try a dynamic memory approach from the book

    struct thread_data *my_thread_data = (struct thread_data *) calloc(0, sizeof(my_thread_data));

    if (my_thread_data == NULL){
        return false;
    }

    // Thank you to https://github.com/cu-ecen-aeld/assignments-3-and-later-Suhas-Reddy-S/tree/master
    // For the example to help me clean this section up.

    my_thread_data->mutex = mutex;
    my_thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    my_thread_data->wait_to_release_ms = wait_to_release_ms;
    my_thread_data->thread_complete_success = false;

 
    s = pthread_create(thread, NULL, (void *)threadfunc, my_thread_data);
    if (s != 0){
        free(my_thread_data);
        return false;
    }

    if (my_thread_data->thread_complete_success == true){
        free(my_thread_data);
    }

    return true;
}

