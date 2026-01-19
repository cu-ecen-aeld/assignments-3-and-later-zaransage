#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "queue.h"
#include "shannon.c"

#define PORT 8080
#define BUFFER_SIZE 1024

int server_socket;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for thread queue access

typedef struct client_thread_s {
    pthread_t thread_id;
    int client_socket;
    SLIST_ENTRY(client_thread_s) entries;  // Queue entry
} client_thread_t;

SLIST_HEAD(client_thread_head, client_thread_s) thread_queue;  // Queue to track active threads

// Signal handler to gracefully shut down the server
void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nShutting down server...\n");

        // Lock the queue and clean up all active threads
        pthread_mutex_lock(&queue_mutex);
        client_thread_t *client_thread, *tmp;
        SLIST_FOREACH_SAFE(client_thread, &thread_queue, entries, tmp) {
            pthread_join(client_thread->thread_id, NULL);  // Join the thread
            SLIST_REMOVE(&thread_queue, client_thread, client_thread_s, entries);  // Remove from queue
            free(client_thread);  // Free the memory
        }
        pthread_mutex_unlock(&queue_mutex);

        close(server_socket);  // Close server socket
        exit(0);
    }
}

// Thread function to write timestamps to "date.txt" every 10 seconds
void *write_date(void *arg) {
    FILE *file = fopen("date.txt", "a");
    if (file == NULL) {
        perror("Error opening date file");
        return NULL;
    }

    while (1) {
        time_t current_time = time(NULL);
        char date_str[26];  // Buffer to store date string
        ctime_r(&current_time, date_str);  // Thread-safe version of ctime()

        pthread_mutex_lock(&queue_mutex);
        fprintf(file, "%s", date_str);
        fflush(file);
        pthread_mutex_unlock(&queue_mutex);

        sleep(10);  // Sleep for 10 seconds before writing again
    }

    fclose(file);
    return NULL;
}

// Thread function to handle client communication
void *handle_client(void *arg) {
    client_thread_t *client_thread = (client_thread_t *)arg;
    int client_socket = client_thread->client_socket;

    FILE *client_file = fopen("client.txt", "a");
    if (client_file == NULL) {
        perror("Error opening client file");
        close(client_socket);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            break;  // Client disconnected or error occurred
        }

        buffer[bytes_received] = '\0';  // Null-terminate the received data

        pthread_mutex_lock(&queue_mutex);
        fprintf(client_file, "%s", buffer);  // Write to the client file
        fflush(client_file);  // Ensure data is written to disk
        pthread_mutex_unlock(&queue_mutex);

        send(client_socket, buffer, bytes_received, 0);  // Echo back the data to the client
    }

    fclose(client_file);
    close(client_socket);

    // Remove this thread from the queue
    pthread_mutex_lock(&queue_mutex);
    SLIST_REMOVE(&thread_queue, client_thread, client_thread_s, entries);
    pthread_mutex_unlock(&queue_mutex);

    free(client_thread);  // Free memory allocated for this thread's data
    return NULL;
}

int main() {
    // Register signal handler for graceful shutdown
    signal(SIGINT, handle_signal);

    // Create a thread to write the date to a file every 10 seconds
    pthread_t date_thread;
    pthread_create(&date_thread, NULL, write_date, NULL);

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind server socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    // Initialize the thread queue
    SLIST_INIT(&thread_queue);

    // Main loop to accept and handle client connections
    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Allocate memory for a new client thread
        client_thread_t *new_client_thread = malloc(sizeof(client_thread_t));
        if (new_client_thread == NULL) {
            perror("Error allocating memory for client thread");
            close(client_socket);
            continue;
        }

        new_client_thread->client_socket = client_socket;

        // Add the new thread to the queue
        pthread_mutex_lock(&queue_mutex);
        SLIST_INSERT_HEAD(&thread_queue, new_client_thread, entries);
        pthread_mutex_unlock(&queue_mutex);

        // Create a new thread to handle the client
        pthread_create(&new_client_thread->thread_id, NULL, handle_client, new_client_thread);
        pthread_detach(new_client_thread->thread_id);  // Detach the thread to clean up automatically
    }

    // Close server socket (though this will not be reached in this design)
    close(server_socket);
    return 0;
}
