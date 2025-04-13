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
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT "9000"
#define FILEPATH "/var/tmp/aesdsocketdata"

volatile sig_atomic_t caught_sigint = 0;
volatile sig_atomic_t caught_sigterm = 0;

bool run_as_daemon = false;

pthread_mutex_t mutex_for_files = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_for_threads = PTHREAD_MUTEX_INITIALIZER;


// Queue
// Can't get queue.h to work. Lets try: http://cslibrary.stanford.edu/103/LinkedListBasics.pdf
struct linked_list_node {
    pthread_t id;
    struct linked_list_node *next;
} struct linked_list_node *thread_list = NULL; // pdf says to start NULL.


// Signal handler from the book
static void signal_handler(int signal_number){
    syslog(LOG_INFO, "Got signal %d", signal_number)
    if (signal_number == SIGINT){
        caught_sigint = 1;
    } else if (signal_number == SIGTERM){
        caught_sigterm = 1;
    }
}


void add_node(pthread_t id){
    // Fist, let me drip this into the heap.
    struct linked_list_node *new_node = malloc(sizeof(struct(linked_list_node)));
    if (!new_node){
        strerror(errno);
        return;
    }
    new_node->id = id;
    new_node->next = NULL; // So sayith the link...

    pthread_mutex_lock(&mutex_for_threads);
    if (!thread_list){
        thread_list = new_node;
    } else {
        struct linked_list_node *current = thread_list;
        while(current->next){
            current = current->next;
        }
        current->next = new_node;
    }
    pthread_mutex_unlock(&mutex_for_threads);
    
}


void remove_node(void){
    pthread_mutex_lock(&mutex_for_threads);
    struct linked_list_node *current = thread_list;
    while(current){
        pthread_join(current->id, NULL);
        struct linked_list_node *temp = current;
        current = current->next;
        free(temp);
    }
    thread_list = NULL;
    pthread_mutex_unlock(&mutex_for_threads);
}


// Timer
// This time, try different thread for time

void *time_thread(void *data) {
    (void)data;
    while (!caught_sigint && !caught_sigterm){
        pthread_mutex_lock(&mutex_for_files);
        FILE *fp = fopen(FILEPATH, "a");
        if (fp) {
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            char buffer[1024];
            strftime(buffer, sizeof(buffer), "timestamp:%a, %d %b %Y %H:%M:%S %z\n", local);
            fclose(fp)            
        }
        pthread_mutex_unlock(&mutex_for_files);

        struct timespec ts = { .tv_sec = 10, .tv_nsec = 0};
        while (nanosleep(&ts, &ts) == -1 && errno == EINTR){
            if (caught_sigint || caught_sigterm) break;
        }
    }

    return NULL;
}

// Thread function for the client

void *client_thread(void *data) {
    int new_fd  = *(int *)data;
    free(data);
    char buffer[1024];
    ssize_t number_of_bytes;

    while (!caught_sigint && !caught_sigterm) { // eah... I don't really like what I did here.
        number_of_bytes = recv(new_fd, buffer, 1024 - 1, 0);
        if (number_of_bytes <= 0){
            strerror(errno);
            break;
        }

        buffer[number_of_bytes] = '\0';
        pthread_mutex_lock(&mutex_for_files);
        FILE *fp = fopen(FILEPATH, "a");
        if (fp){
            if (fputs(buffer, fp) < 0){
                strerror(errno);
            }
            if (fflush(fp) != 0){
                strerror(errno);
            }
            fclose(fp);
        }
        else {
            strerror(errno);
        }
        pthread_mutex_unlock(&mutex_for_files);

        pthread_mutex_lock(&mutex_for_files);
        fp = fopen(FILEPATH, "r");
        if (fp){
            while ((number_of_bytes = fread(buffer, 1, 1024, fp)) > 0){
                if (send(new_fd, buffer, number_of_bytes, 0)){
                    fclose(fp);
                    break;
                }
            }
            fclose(fp);
        }
        pthread_mutex_unlock(&mutex_for_files);

        if (strchr(buffer, '\n')){
            break;
        }
        close(new_fd);
        return NULL;
    }
}


int main(int argc, char *argv[]){
    // Socket handling
    int sockfd;
    struct addrinfo hints, *serverinfo, *p;
    socklen_t sin_size;
    int yes = 1;
    int rv;
    
    for (int i =1; i < argc; i++){
        if (strcmp(argv[i], "-d") == 0){
            run_as_daemon = true;
            break;
        }
    }

    openlog(NULL, 0, LOG_USER);

    // Queue handling
    // Let me replace with a linked list
    /* Right from the material example .. lets try it*/

    if (run_as_daemon) {
        pid_t pid = fork();

        if (pid < 0) {
            exit(1);
        }

        if (pid > 0) {
            exit(0);
        }

        setsid();

        pid_t pid2 = fork();

        if (pid2 < 0) {
            exit(1);
        }

        if (pid2 > 0) {
            exit(0);
        }
        umask(0);

        if (chdir("/") != 0) {
            syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        }

      }

    /*A horrible smattering of beej.us, Linux Systems Programming and some Googling errors*/
    struct sigaction new_action = {.sa_handler = signal_handler}
    sigemptyset(&new_action.sa_mask);

    if (sigaction(SIGTERM, &new_action, NULL) != 0){
        closelog();
        exit(1);
    }

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
    if(pthread_create(&timestamp_id, NULL, time_thread, NULL) != 0){
        perror("Timestamp thread");
        close(sockfd);
        exit(1);
    }
    // More queue stuff
    add_node(timestamp_tid);

    printf("Starting While Loop:\n");
    while (!caught_sigint && !caught_sigterm) {
        struct sockaddr_storage connect_addr;
        socket_t sin_size = sizeof(connect_addr);
        int *new_fd = malloc(sizeof(int));

        if(!new_fd){
            sleep(1);
            continue;
        }

        int *new_fd = accept(sockfd, (struct sockaddr *)&connect_addr, &sin_size);
        if (*new_fd == -1){
            perror("accept");
            free(new_fd);
            sleep(1); // For some reason I need this....
            continue;
        }

        if (connect_addr.ss_family == AF_INET){
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&connect_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, s, sizeof(s));
            printf("Accepted connection from %s\n", s);

        }

        syslog(LOG_INFO, "Accepted connection from %s", s);

        pthread_t client_tid;
        if (pthread_create(&client_tid, NULL, client_thread, new_fd) != 0) {
            perror("pthread_create");
            close(*new_fd);
            free(new_fd);
            continue;
        }
        add_node(client_tid);
        }
        remove_node();
        close(sockfd);
        pthread_mutex_destroy(&mutex_for_files);
        pthread_mutex_destroy(&mutex_for_threads);
        remove(FILEPATH);
        closelog();
        return 0;
}