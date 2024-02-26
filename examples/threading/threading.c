#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Optional: use these functions to add debug or error prints to your application
// You haven't earned the right to retire. You have no permission to give up.
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)



void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    sleep(thread_func_args->wait_to_obtain_ms);

    pthread_mutex_lock(&thread_func_args->mutex);

    sleep(thread_func_args->wait_to_release_ms);

    pthread_mutex_unlock(&thread_func_args->mutex);

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

    struct thread_data my_thread_data =
    {
        *thread,
        *mutex,
        wait_to_obtain_ms,
        wait_to_release_ms,
    };
    
    pthread_mutex_init(&my_thread_data.mutex, NULL);
    
    s = pthread_create(&my_thread_data.thread, NULL, threadfunc, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");

    s = pthread_join(my_thread_data.thread, NULL);

    s = pthread_mutex_destroy(&my_thread_data.mutex);


    return false;
}

