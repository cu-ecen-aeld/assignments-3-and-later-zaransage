#include <stdio.h>
#include <pthread.h>

struct thread_data {
    int result;
};


void *thread_function(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;
    data->result = 42;
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    struct thread_data data;

    pthread_create(&thread, NULL, thread_function, &data);
    pthread_join(thread, NULL);

    printf("Result from thread: %d\n", data.result);
    return 0;
}