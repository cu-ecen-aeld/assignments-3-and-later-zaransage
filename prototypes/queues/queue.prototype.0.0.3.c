#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "queue.h"
#include "shannon.c"

pthread_mutex_t mutex;
typedef struct slist_data_s slist_data_t;


struct slist_data_s {
    pthread_t value;
    bool result;
    SLIST_ENTRY(slist_data_s) entries;
} *np_temp;


struct shared_thread_data {
    pthread_mutex_t *mutex;
    bool result;
    pthread_t thread_id;
    time_t time;
};


void myTime() {
    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %d %b %y %r %z", local);

    FILE *fp = fopen("/tmp/time.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
    }

    fprintf(fp, "%s\n", buffer);
    fclose(fp);
}

void *thread_function_read(void *data) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)data;  
    int s;
    
    if (pthread_mutex_init(&mutex, NULL) != 0) {
    perror("Mutex initialization failed");
    exit(1);
    }

    thread_data->thread_id = pthread_self();
    //printf("Thread ID from function: %lu\n", (unsigned long)pthread_self());
    //myTime();
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main (int argc, char *argv[]) {

    pthread_t thread1, thread2;

    struct shared_thread_data data1, data2;
    //struct shared_thread_data *data = (struct thread_data *) calloc(0, sizeof(data));

    slist_data_t *datap=NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);

    int a, b;

    printf("Mutex\n");
    pthread_mutex_init(&mutex, NULL);

    printf("Thread create\n");
    a = pthread_create(&thread1, NULL, thread_function_read, &data1);
    printf("Result from thread 1: %d\n", a);

    datap = malloc(sizeof(slist_data_t));
    datap->value = data1.thread_id;
    SLIST_INSERT_HEAD(&head, datap, entries);

    b = pthread_create(&thread2, NULL, thread_function_read, &data2);
    printf("Result from thread 2: %d\n", b);
    
    datap = malloc(sizeof(slist_data_t));
    datap->value = data2.thread_id;
    SLIST_INSERT_HEAD(&head, datap, entries);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    SLIST_FOREACH_SAFE(datap, &head, entries, np_temp) {
    printf("Thread ID: %lu\n", datap->value);
    }


    pthread_mutex_destroy(&mutex);

}