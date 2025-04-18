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
    
    pthread_mutex_lock(&mutex);

    //printf("Thread ID from function: %lu\n", (unsigned long)pthread_self());
    //myTime();
    pthread_mutex_unlock(&mutex);

    return NULL;
}


void queueAddItem(pthread_t tid) {

    slist_data_t *datap=NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);

    datap = malloc(sizeof(slist_data_t));
    datap->value = tid;
    SLIST_INSERT_HEAD(&head, datap, entries);
    SLIST_FOREACH_SAFE(datap, &head, entries, np_temp) {
    printf("Thread ID: %lu\n", datap->value);
    }

}

int main (int argc, char *argv[]) {

    pthread_t thread1, thread2;

    struct shared_thread_data data1;
    //struct shared_thread_data *data = (struct thread_data *) calloc(0, sizeof(data));


    int a, b;

    pthread_mutex_init(&mutex, NULL);

    a = pthread_create(&thread1, NULL, thread_function_read, &data1);
    queueAddItem(thread1);

    b = pthread_create(&thread2, NULL, thread_function_read, &data1);
    queueAddItem(thread2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex);

}