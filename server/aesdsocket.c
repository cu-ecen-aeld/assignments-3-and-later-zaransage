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
};
struct linked_list_node *thread_list = NULL; // pdf says to start NULL.


// Signal handler from the book
static void signal_handler(int signal_number) {
    syslog(LOG_INFO, "Caught signal %d", signal_number);
    if (signal_number == SIGINT) {
        caught_sigint = 1;
    } else if (signal_number == SIGTERM) {
        caught_sigterm = 1;
    }
}


void add_node(pthread_t id) {
    struct linked_list_node *new_node = malloc(sizeof(struct linked_list_node));
    if (!new_node) {
        syslog(LOG_ERR, "Failed to allocate thread node");
        return;
    }
    new_node->id = id;
    new_node->next = NULL; // So sayith the link...

    pthread_mutex_lock(&mutex_for_threads);
    syslog(LOG_DEBUG, "Adding thread %lu to list", (unsigned long)id);
    if (!thread_list) {
        thread_list = new_node;
    } else {
        struct linked_list_node *current = thread_list;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
    pthread_mutex_unlock(&mutex_for_threads);
}


void remove_node(void) {
    pthread_mutex_lock(&mutex_for_threads);
    struct linked_list_node *current = thread_list;
    while (current) {
        syslog(LOG_DEBUG, "Joining thread %lu", (unsigned long)current->id);
        pthread_join(current->id, NULL);
        struct linked_list_node *temp = current;
        current = current->next;
        free(temp);
    }
    thread_list = NULL;
    pthread_mutex_unlock(&mutex_for_threads);
}


// timestampr
// This timestamp, try different thread for timestamp

void *timestamp_thread(void *arg) {
    (void)arg;
    while (!caught_sigint && !caught_sigterm) {
        syslog(LOG_DEBUG, "Timestamp thread attempting to lock mutex");
        pthread_mutex_lock(&mutex_for_files);
        syslog(LOG_DEBUG, "Timestamp thread opening %s", FILEPATH);
        FILE *fp = fopen(FILEPATH, "a");
        if (fp) {
            syslog(LOG_DEBUG, "Timestamp thread writing to %s", FILEPATH);
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char buffer[1024];
            strftime(buffer, sizeof(buffer), "timestamp:%a, %d %b %Y %H:%M:%S %z\n", tm_info);
            if (fputs(buffer, fp) < 0) {
                syslog(LOG_ERR, "Timestamp thread failed to write: %s", strerror(errno));
            }
            if (fflush(fp) != 0) {
                syslog(LOG_ERR, "Timestamp thread failed to flush: %s", strerror(errno));
            }
            fclose(fp);
        } else {
            syslog(LOG_ERR, "Timestamp thread failed to open %s: %s", FILEPATH, strerror(errno));
        }
        pthread_mutex_unlock(&mutex_for_files);
        syslog(LOG_DEBUG, "Timestamp thread mutex unlocked");

        struct timespec ts = { .tv_sec = 10, .tv_nsec = 0 };
        while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
            if (caught_sigint || caught_sigterm) break;
        }
    }
    syslog(LOG_DEBUG, "Timestamp thread exiting");
    return NULL;
}


// Thread function for the client
void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[1024];
    ssize_t bytes_received;

    syslog(LOG_DEBUG, "Client thread started for fd %d", client_fd);

    while (!caught_sigint && !caught_sigterm) { // eah... I don't really like what I did here.
        bytes_received = recv(client_fd, buffer, 1024 - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received < 0) {
                syslog(LOG_ERR, "Receive error on fd %d: %s", client_fd, strerror(errno));
            } else {
                syslog(LOG_INFO, "Client fd %d disconnected", client_fd);
            }
            break;
        }

        buffer[bytes_received] = '\0';
        syslog(LOG_DEBUG, "Received %zd bytes on fd %d", bytes_received, client_fd);

        pthread_mutex_lock(&mutex_for_files);
        FILE *fp = fopen(FILEPATH, "a");
        if (fp) {
            if (fputs(buffer, fp) < 0) {
                syslog(LOG_ERR, "Client thread failed to write: %s", strerror(errno));
            }
            if (fflush(fp) != 0) {
                syslog(LOG_ERR, "Client thread failed to flush: %s", strerror(errno));
            }
            fclose(fp);
            syslog(LOG_DEBUG, "Client thread wrote to %s", FILEPATH);
        } else {
            syslog(LOG_ERR, "Client thread failed to open %s: %s", FILEPATH, strerror(errno));
        }
        pthread_mutex_unlock(&mutex_for_files);

        pthread_mutex_lock(&mutex_for_files);
        fp = fopen(FILEPATH, "r");
        if (fp) {
            while ((bytes_received = fread(buffer, 1, 1024, fp)) > 0) {
                if (send(client_fd, buffer, bytes_received, 0) < 0) {
                    syslog(LOG_ERR, "Send error on fd %d: %s", client_fd, strerror(errno));
                    fclose(fp);
                    break;
                }
            }
            fclose(fp);
            syslog(LOG_DEBUG, "Client thread sent file contents");
        } else {
            syslog(LOG_ERR, "Client thread failed to read %s: %s", FILEPATH, strerror(errno));
        }
        pthread_mutex_unlock(&mutex_for_files);

        if (strchr(buffer, '\n')) {
            syslog(LOG_DEBUG, "Newline received on fd %d, closing", client_fd);
            break;
        }
    }

    close(client_fd);
    syslog(LOG_DEBUG, "Client thread for fd %d exiting", client_fd);
    return NULL;
}


int main(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *serverinfo, *p;
    int yes = 1;
    int rv;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            run_as_daemon = true;
            break;
        }
    }

    openlog(NULL, 0, LOG_USER);
    syslog(LOG_INFO, "Starting aesdsocket%s", run_as_daemon ? " in daemon mode" : "");

    // Queue handling
    // Let me replace with a linked list
    /* Right from the material example .. lets try it*/

    if (run_as_daemon) {
        syslog(LOG_DEBUG, "Daemonizing...");
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "First fork failed: %s", strerror(errno));
            exit(1);
        }
        if (pid > 0) {
            exit(0);
        }
        setsid();
        pid_t pid2 = fork();
        if (pid2 < 0) {
            syslog(LOG_ERR, "Second fork failed: %s", strerror(errno));
            exit(1);
        }
        if (pid2 > 0) {
            exit(0);
        }
        umask(0);
        if (chdir("/") != 0) {
            syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        }
        int dev_null = open("/dev/null", O_RDWR);
        if (dev_null < 0) {
            syslog(LOG_ERR, "Failed to open /dev/null: %s", strerror(errno));
        } 
    }

    /*A horrible smattering of beej.us, Linux Systems Programming and some Googling errors*/
    struct sigaction new_action = { .sa_handler = signal_handler };
    sigemptyset(&new_action.sa_mask);
    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        syslog(LOG_ERR, "Failed to register SIGTERM handler: %s", strerror(errno));
        closelog();
        exit(1);
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        syslog(LOG_ERR, "Failed to register SIGINT handler: %s", strerror(errno));
        closelog();
        exit(1);
    }
    syslog(LOG_DEBUG, "Signal handlers registered");

    // Original socket setup I think will work. Its my threads which are broken...

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    syslog(LOG_DEBUG, "Calling getaddrinfo");
    if ((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0) {
        syslog(LOG_ERR, "getaddrinfo failed: %s", gai_strerror(rv));
        closelog();
        exit(1);
    }

    for (p = serverinfo; p != NULL; p = p->ai_next) {
        syslog(LOG_DEBUG, "Attempting socket creation for family %d", p->ai_family);
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            syslog(LOG_ERR, "Socket creation failed: %s", strerror(errno));
            continue;
        }

        syslog(LOG_DEBUG, "Setting SO_REUSEADDR for fd %d", sockfd);
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            syslog(LOG_ERR, "Setsockopt failed: %s", strerror(errno));
            close(sockfd);
            continue;
        }

        syslog(LOG_DEBUG, "Binding fd %d", sockfd);
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            syslog(LOG_ERR, "Bind failed: %s", strerror(errno));
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(serverinfo);
    
    /*Borrowed the lower two if conditions heavily from beej.us*/

    if (p == NULL) {
        syslog(LOG_ERR, "Failed to bind to any address");
        closelog();
        exit(1);
    }

    syslog(LOG_DEBUG, "Listening on fd %d", sockfd);
    if (listen(sockfd, 10) == -1) {
        syslog(LOG_ERR, "Listen failed: %s", strerror(errno));
        close(sockfd);
        closelog();
        exit(1);
    }
    syslog(LOG_INFO, "Server listening on port %s", PORT);

    pthread_t timestamp_tid;
    syslog(LOG_DEBUG, "Creating timestamp thread");
    if (pthread_create(&timestamp_tid, NULL, timestamp_thread, NULL) != 0) {
        syslog(LOG_ERR, "Failed to create timestamp thread: %s", strerror(errno));
        close(sockfd);
        closelog();
        exit(1);
    }
    add_node(timestamp_tid);
    syslog(LOG_DEBUG, "Timestamp thread created");

    while (!caught_sigint && !caught_sigterm) {
        struct sockaddr_storage connect_addr;
        socklen_t sin_size = sizeof(connect_addr);
        int *new_fd = malloc(sizeof(int));
        if (!new_fd) {
            syslog(LOG_ERR, "Failed to allocate client fd");
            sleep(1);
            continue;
        }

        syslog(LOG_DEBUG, "Waiting for connection");
        *new_fd = accept(sockfd, (struct sockaddr *)&connect_addr, &sin_size);
        if (*new_fd == -1) {
            syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
            free(new_fd);
            sleep(1);
            continue;
        }

        char s[INET6_ADDRSTRLEN]; // Why on earth do I need this

        if (connect_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&connect_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, s, sizeof(s));
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&connect_addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, s, sizeof(s)); // Sigh. Next time, just do both protocols.
        }
        syslog(LOG_INFO, "Accepted connection from %s", s);

        pthread_t client_tid;
        syslog(LOG_DEBUG, "Creating client thread for fd %d", *new_fd);
        if (pthread_create(&client_tid, NULL, client_thread, new_fd) != 0) {
            syslog(LOG_ERR, "Failed to create client thread: %s", strerror(errno));
            close(*new_fd);
            free(new_fd);
            continue;
        }
        add_node(client_tid);
        syslog(LOG_DEBUG, "Client thread created for fd %d", *new_fd);
    }

    syslog(LOG_INFO, "Shutting down...");
    remove_node();
    close(sockfd);
    pthread_mutex_destroy(&mutex_for_files);
    pthread_mutex_destroy(&mutex_for_threads);
    if (remove(FILEPATH) != 0 && errno != ENOENT) {
        syslog(LOG_ERR, "Failed to remove %s: %s", FILEPATH, strerror(errno));
    }
    closelog();
    return 0;
}