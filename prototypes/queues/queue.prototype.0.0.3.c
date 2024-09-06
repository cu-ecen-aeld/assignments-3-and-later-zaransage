#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "queue.h"
#include "shannon.c"


typedef struct slist_data_s slist_data_t;

struct slist_data_s {
    int value;
    bool result;
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

    //thread_data->thread_id = pthread_self();
    // Time and writes go here eventually

    thread_data->thread_id = 42;
    
    s = pthread_mutex_unlock(thread_data->mutex);
    if (s != 0) {
        thread_data->result = false;
    }

    thread_data->result = true;

    return arg;
}

void slist(void *data) {
  slist_data_t *datap=NULL;

  SLIST_HEAD(slisthead, slist_data_s) head;
  SLIST_INIT(&head);

  datap = malloc(sizeof(slist_data_t));
  
  SLIST_INSERT_HEAD(&head, datap, entries);
  datap->value = (int) (randshannon() * 1000.);

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
    pthread_mutex_t mutex;

    struct shared_thread_data data;
    //struct shared_thread_data *data = (struct thread_data *) calloc(0, sizeof(data));

    slist_data_t *datap=NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);

    int a, b;

    // I need the thread ID to add to a queue
    // Then I need to read the thread ID status
    // Then I need to join
    // Not just try to randomly join everything.

    a = pthread_create(&thread1, NULL, thread_function_read, &data);
    //printf("Result from thread 1: %d\n", a);
    a = pthread_self();

    datap = malloc(sizeof(slist_data_t));
    datap->value = a;
    SLIST_INSERT_HEAD(&head, datap, entries);

    b = pthread_create(&thread2, NULL, thread_function_read, &data);
    //printf("Result from thread 2: %d\n", b);
    b = pthread_self();

    datap = malloc(sizeof(slist_data_t));
    datap->value = b;
    SLIST_INSERT_HEAD(&head, datap, entries);

    SLIST_FOREACH(datap, &head, entries) {
    printf("%d\n", datap->value);
  }

}