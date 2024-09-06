#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "queue.h"
#include "shannon.c"


typedef struct slist_data_s slist_data_t;

struct slist_data_s {
    int value;
    SLIST_ENTRY(slist_data_s) entries;
} *np_temp;


struct shared_thread_data {
    pthread_mutex_t *mutex;
    bool result;
    int thread_id;
};


void *thread_function_read(void *arg) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)arg;  
    int s;
    s = pthread_mutex_lock(thread_data->mutex);
    if (s != 0) {
        thread_data->result = false;
    }

    thread_data->thread_id = pthread_self();
    
    s = pthread_mutex_unlock(thread_data->mutex);
    if (s != 0) {
        thread_data->result = false;
    }

    thread_data->result = true;

    return arg;
}

void slist(int tid) {
  slist_data_t *datap=NULL;

  SLIST_HEAD(slisthead, slist_data_s) head;
  SLIST_INIT(&head);

  datap = malloc(sizeof(slist_data_t));
  datap->value = tid;
  
  SLIST_INSERT_HEAD(&head, datap, entries);
  
  SLIST_FOREACH_SAFE(datap, &head, entries, np_temp) {
    printf("Thread ID: %d\n", datap->value);
  }

  while (!SLIST_EMPTY(&head)) {
    datap = SLIST_FIRST(&head);
    SLIST_REMOVE_HEAD(&head, entries);
    free(datap);
  }

}


int main (int argc, char *argv[]) {

    pthread_t thread1, thread2;
    struct shared_thread_data data;

    pthread_create(&thread1, NULL, thread_function_read, &data);
    printf("Result from thread 1: %d\n", data.thread_id);
    slist(data.thread_id);
    pthread_create(&thread2, NULL, thread_function_read, &data);
    slist(data.thread_id);
    printf("Result from thread 2: %d\n", data.thread_id);
    printf("\n%d\n", data.thread_id);
    return 0;
}