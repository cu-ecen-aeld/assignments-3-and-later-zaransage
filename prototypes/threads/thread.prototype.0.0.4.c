#include <stdio.h>
#include <pthread.h>

struct shared_thread_data {
    int result;
};


void *thread_function_read(void *arg) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)arg;    
    thread_data->result = 1;
}

void *thread_function_write(void *arg) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)arg;
    thread_data->result = 2;

}


int main() {
    pthread_t thread;
    struct shared_thread_data data;

    pthread_create(&thread, NULL, thread_function_read, &data);

    printf("Result from thread: %d\n", data.result);

    return 0;
}
