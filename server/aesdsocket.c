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




int main( int argc, char *argv[]){

  int sockfd, new_fd;
  struct addrinfo hints, *serverinfo, *p;
  struct SOCKADDR *ai_addr;
  struct sockaddr_storage connect_addr;
  socklen_t sin_size;
  struct sigaction sig_action;
  int yes = 1;
  int rv;
  char s[INET6_ADDRSTRLEN];

  FILE *fp = fopen(FILEPATH, "w");

  openlog(NULL,0,LOG_USER);

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // Everything I read says I am missing a loop here.

for (p = serverinfo; p != NULL; p = p->ai_next){

  if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
    perror("Unable to open socket");
    syslog(LOG_ERR,"Unable to open socket");
     continue;
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("Unable to open socket: opt error");
    syslog(LOG_ERR,"Socket opt error");
    return 1;
  }

  if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    close(sockfd);
    perror("Unable to bind socket");
    syslog(LOG_ERR,"Unable to bind socket");
    continue;
  };
    break;
  }

  freeaddrinfo(serverinfo);

  /*Heavily borrowed from beej.us*/

  if (p == NULL){
    fprintf(stderr, "failed to bind");
    perror("Unable to bind socket");
    syslog(LOG_ERR,"Unable to bind socket");
    return 1;

  }

  if (listen(sockfd, 10) == -1){
    fprintf(stderr, "failed to bind");
    perror("Unable to bind socket");
    syslog(LOG_ERR,"Unable to bind socket");
    return 1;
  }

  while(1){
    sin_size = sizeof connect_addr;
    new_fd = accept(sockfd, (struct sockaddr *) &connect_addr, &sin_size);

    if (new_fd < 0){
        perror("Unable to accept socket");
        syslog(LOG_ERR,"Unable to accept socket");
        return 1;
        continue;
    }

    if (connect_addr.ss_family == AF_INET){
        struct sockaddr_in *ipv4 = (struct sockaddr_in *) &connect_addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), s, sizeof s);

        printf("Accepted connection from %s\n",s);
        syslog(LOG_INFO,"Accepted connection from %s\n",s);
    }

    if (!fork()){
        close(sockfd);
        if (send(new_fd, "connection accepted", 13, 0) == -1){
            perror("failed to send");
        }
        close(new_fd);
        perror("Unable to send data to socket");
        syslog(LOG_ERR,"Unable to send data to socket");
        return 0;
    }

    close(sockfd);
    syslog(LOG_INFO, "Closed connection from %s\n" ,s);

  }

    close(sockfd);
    remove(FILEPATH);
    closelog();
    return 0;
}