#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Shared counter and mutex
int counter = 0;
pthread_mutex_t mutex;

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

// Thread function
void *thread_function(void *arg) {
    // Lock the mutex before accessing the shared counter
    pthread_mutex_lock(&mutex);

    // Increment the counter
    counter++;
    printf("Thread ID: %lu, Counter: %d\n", (unsigned long)pthread_self(), counter);
    myTime();

    // Unlock the mutex after accessing the shared counter
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(1);
    }

    // Create the first thread
    if (pthread_create(&thread1, NULL, thread_function, NULL) != 0) {
        perror("Error creating thread 1");
        exit(1);
    }

    // Create the second thread
    if (pthread_create(&thread2, NULL, thread_function, NULL) != 0) {
        perror("Error creating thread 2");
        exit(1);
    }

    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    // Print final counter value
    printf("Final Counter Value: %d\n", counter);

    return 0;
}
