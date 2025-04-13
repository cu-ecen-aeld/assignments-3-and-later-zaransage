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
#include <fnctl.h>
#include "queue.h"

#define PORT "9000"
#define FILEPATH "/var/tmp/aesdsocketdata"

#define RUNNING 0
#define COMPLETED 1

volatile sig_atomic_t caught_sigint = 0;
volatile sig_atomic_t caught_sigterm = 0;

bool run_as_daemon = false;

pthread_mutex_t mutex_for_files = MUTEX;
pthread_mutex_t mutex_for_threads = MUTEX;

// Signal handler from the book
static void signal_handler(int signal_number){
    syslog(LOG_INFO, "Got signal %d", signal_number)
    if (signal_number == SIGINT){
        caught_sigint = 1;
    } else if (signal_number == SIGTERM){
        caught_sigterm = 1;
    }
}

// Timer
// This time, try different thread for time

void *time_thread(void *data) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)data;  
    while (!caught_sigint && !caught_sigterm){
        pthread_mutex_lock(&mutex_for_files);
        FILE *fp = fopen(FILEPATH, "a+");
        if (fp == NULL) {
            time_t now;
            struct tm *local = localtime(&now);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "timestamp:%a, %d %b %Y %r %z", local);
            fclose(fp)            
        }
        pthread_mutex_unlock(&mutex_for_files);

        struct timespec ts = { .tv_sec = 1, .tv_nsec = 0};
        while (nanosleep(&ts, &ts) == -1 && errno == EINTR){
            if (caught_sigint || caught_sigterm) break;
        }
    }

    return NULL;
}

// Thread function for the client

struct shared_thread_data {
    pthread_mutex_t *mutex;
    int status;
};

void *client_thread(void *data) {
    struct shared_thread_data *thread_data = (struct shared_thread_data *)data;  
    thread_data->status = RUNNING;

    while (!caught_sigint && !caught_sigterm) { // eah... I don't really like what I did here.
        pthread_mutex_lock(&mutex);
        myTime();
        pthread_mutex_unlock(&mutex);
        sleep(10);
    }

    pthread_mutex_lock(&mutex);
    if(pthread_mutex_lock(&mutex) != 0) {
        syslog(LOG_ERR, "Mutex lock failed");
        printf("myTime thread lock error\n");
    }
    thread_data->status = COMPLETED;
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);
}

// Queue
// Can't get queue.h to work. Lets try: http://cslibrary.stanford.edu/103/LinkedListBasics.pdf
struct linked_list_node{
    pthread_t id;
    struct node *next
} struct node *thread_list = NULL; // pdf says to start NULL.

void add_node(){
    // Memory allocate
    // Add data to struct
    // Point one node to the next
    return;
}

void remove_node(){
    // Might have to seek and remove
    // Update link? Set data to NULL?
    return;
}


int main(int argc, char *argv[]){
    // Socket handling
    int sockfd, new_fd;
    struct addrinfo hints, *serverinfo, *p;
    struct sockaddr_storage connect_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    int rv;

    // Thread handling
    pthread_t t1;
    void *res;
    int s_t;
    char data;

    // Queue handling
    // Let me replace with a linked list

    FILE *fp = fopen(FILEPATH, "a+");
    openlog(NULL, 0, LOG_USER);

    /* Right from the material example .. lets try it*/

    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler=signal_handler;

    if(sigaction(SIGTERM, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGTERM", errno, strerror(errno));
    }

    if(sigaction(SIGINT, &new_action, NULL)){
        printf("Error %d (%s) registereing for SIGINT", errno, strerror(errno));
    }

      /*Lets try some of the forking code from class 2*/

    for (int i =1; i < argc; i++){
        if (strcmp(argv[i], "-d") == 0){
            run_as_daemon = true;
            break;
        }
    }

    if (run_as_daemon){
        pid_t pid;
            pid = fork();

        if (pid < 0){
          perror("fork");
          exit(-1);
        }

        if (pid > 0){
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            exit(0); // A thank you to https://github.com/cu-ecen-aeld/assignments-3-and-later-asabbagh4 for this one exit to fix everything.
        }
        setsid();
        umask(0);
        syslog(LOG_DEBUG, "Daemon working")
    }

    /*A horrible smattering of beej.us, Linux Systems Programming and some Googling errors*/
    struct sigaction new_action;    


    // Original socket setup I think will work. Its my threads which are broken...
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = serverinfo; p != NULL; p = p->ai_next){
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("Unable to open socket");
            syslog(LOG_ERR, "Unable to open socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
            perror("Unable to set socket options");
            syslog(LOG_ERR, "Socket option error");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("Unable to bind socket");
            syslog(LOG_ERR, "Unable to bind socket");
            continue;
        }
        break; 
    }

    freeaddrinfo(serverinfo);

/*Borrowed the lower two if conditions heavily from beej.us*/

    if (p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, 10) == -1){
        perror("listen");
        close(sockfd);
        exit(1);
    }

/* Let me start time thread*/
    pthread_t timestamp_id;
    if(pthread_create(&timestamp_id, NULL,) != 0){
        perror("Timestamp thread");
        close(sockfd);
        exit(1);
    }
    // More queue stuff later


    printf("Starting While Loop:\n");
    while (!caught_sigint && !caught_sigterm) {
        socket_t sin_size = sizeof connect_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&connect_addr, &sin_size);

        if (*new_fd == -1){
            perror("accept");
            free(new_fd);
            continue;
        }

        if (connect_addr.ss_family == AF_INET){
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&connect_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, s, sizeof(s));
            printf("Accepted connection from %s\n", s);

        }

        syslog(LOG_INFO, "Accepted connection from %s", s);

        slist_data_t *new_thread = malloc(sizeof(slist_data_t));
        pthread_t client_tid;
        if (pthread_create(&client_tid, NULL, client_thread, new_thread) != 0) {
            perror("pthread_create");
            close(*new_thread)
            free(new_thread);
            continue;
        }
        // This is the broken spot still. I need to replace this part later.
        pthread_mutex_lock(&mutex);
        SLIST_INSERT_HEAD(&thread_queue, new_thread, entries);
        pthread_mutex_unlock(&mutex);

        slist_data_t *iter, *temp;
        pthread_mutex_lock(&mutex);

        SLIST_FOREACH_SAFE(iter, &thread_queue, entries, temp) {
            if (iter->status == COMPLETED) {
                pthread_join(iter->thread_id, NULL);
                printf("Joining thread %lu\n", (unsigned long) iter->thread_id);

                SLIST_REMOVE(&thread_queue, iter, slist_data_s, entries);
                free(iter);
            }
        }

        pthread_mutex_unlock(&mutex);

        // Thread to here - This below somehow goes into client thread now. 
        if (!fork()){ 
            close(sockfd);

            char buffer[1024];
            int number_of_bytes;

            while ((number_of_bytes = recv(new_fd, buffer, sizeof(buffer) -1, 0)) > 0){
                buffer[number_of_bytes] = '\0';
                fputs(buffer, fp);
                if (strchr(buffer, '\n')) break;
            }

            fflush(fp);
            
            rewind(fp);
            while ((number_of_bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0){
                send(new_fd, buffer, number_of_bytes, 0);
            }

            fclose(fp);
            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

   
    close(sockfd);
    closelog();

    if (caught_sigint) {
        printf("\nCaught SIGINT, exiting.\n");
        syslog(LOG_ERR, "Caught SIGINT, exiting.");
        remove(FILEPATH);
    }
    if (caught_sigterm) {
        printf("\nCaught SIGTERM, exiting.\n");
        syslog(LOG_ERR, "Caught SIGTERM, exiting.");
        remove(FILEPATH);
    }

    remove(FILEPATH);
    pthread_mutex_destroy(&mutex);
    return 0;
}