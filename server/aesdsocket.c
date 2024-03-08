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


#define PORT "9000"
#define FILEPATH "/var/tmp/aesdsocketdata"

bool caught_sigint = false;
bool caught_sigterm = false;

static void signal_handler(int signal_number){
    if (signal_number == SIGINT){
        caught_sigint = 1;
    } else if (signal_number == SIGTERM){
        caught_sigterm = 1;
    }
}

int main(int argc, char *argv[]){
    int sockfd, new_fd;
    struct addrinfo hints, *serverinfo, *p;
    struct sockaddr_storage connect_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    int rv;

    FILE *fp = fopen(FILEPATH, "a+");
    openlog(NULL, 0, LOG_USER);

    /* Right from the material example .. lets try it*/

    struct sigaction new_action;
    bool success = true;
    
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler=signal_handler;

    if(sigaction(SIGTERM, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGTERM", errno, strerror(errno));
    }

    if(sigaction(SIGINT, &new_action, NULL)){
        printf("Error %d (%s) registereing for SIGINT", errno, strerror(errno));
    }


    /*A horrible smattering of beej.us, Linux Systems Programming and some Googling errors*/

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
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
            return 1;
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
        return 1;
    }

    if (listen(sockfd, 10) == -1){
        perror("listen");
        return 1;
    }

    while (1) {
        if (caught_sigint || caught_sigterm) {
            break;
        }
        sin_size = sizeof connect_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&connect_addr, &sin_size);

        if (new_fd == -1){
            perror("accept");
            continue;
        }

        if (connect_addr.ss_family == AF_INET){
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&connect_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, s, sizeof s);
        }

        printf("Accepted connection from %s\n", s);
        syslog(LOG_INFO, "Accepted connection from %s", s);

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
    return 0;
}