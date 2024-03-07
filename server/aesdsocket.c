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

#define PORT "9000"
#define FILEPATH "/var/tmp/aesdsocketdata"


int main(int argc, char *argv[]){
    int sockfd, new_fd;
    struct addrinfo hints, *serverinfo, *p;
    struct sockaddr_storage connect_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    int rv;

    FILE *fp = fopen(FILEPATH, "w");
    openlog(NULL, 0, LOG_USER);

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

            if (send(new_fd, "connection accepted", strlen("connection accepted") + 1, 0) == -1){
                perror("send");
            }

            while ((number_of_bytes = recv(new_fd, buffer, sizeof(buffer) -1, 0)) > 0){
                buffer[number_of_bytes] = '\n';
                fprintf(fp, "%s", buffer);
            }

            if (number_of_bytes < 0){
                perror("Not getting bytes");
            }

            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

   
    close(sockfd);
    //remove(FILEPATH);
    closelog();
    return 0;
}