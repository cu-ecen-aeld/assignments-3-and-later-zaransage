#include <stdio.h>
#include <pthread.h>


struct thread_data {
    int result;
};


void *thread_function(void *arg) {
    int result = 42;
    pthread_exit((void *) result);
}


int main() {
    pthread_t thread;
    void *thread_result;

    pthread_create(&thread, NULL, thread_function, NULL);

    pthread_join(thread, &thread_result);

    int result = (int)thread_result;
    printf("Result from therad: %d\n", result);

    return 0;
}