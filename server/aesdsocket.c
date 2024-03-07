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

#define PORT 9000




int main( int argc, char *argv[]){

  int sockfd, new_fd;
  struct addrinfo ai, *serverinfo;
  struct SOCKADDR *ai_addr;
  struct sockaddr_storage connect_addr;
  socklen_t sin_size;
  struct sigaction sig_action;
  int yes = 1;
  int rv;
  char s[INET6_ADDRSTRLEN];

  FILE *fp = fopen("/var/tmp/aesdsocketdata", "w");

  openlog(NULL,0,LOG_USER);

  memset(&ai, 0, sizeof ai);

  if ((rv = getaddrinfo(NULL, PORT, &ai, &serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  if ((sockfd = socket(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE)) == -1){
    perror("Unable to open socket");
    syslog(LOG_ERR,"Unable to open socket");
    return 1;
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("Unable to open socket: opt error");
    syslog(LOG_ERR,"Socket opt error");
    return 1;
  }

  if (bind(sockfd, ai_addr, sizeof(ai_addr)) == -1 {
    close(sockfd);
    perror("Unable to bind socket");
    syslog(LOG_ERR,"Unable to bind socket");
    return 1;
  });


  freeaddrinfo(serverinfo);

  // I think my handling of the flags needs to change

  while(1){
    sin_size = sizeof connect_addr;
    new_fd = accept(sockfd, (struct sockaddr *) &connect_addr, &sin_size);

    if (new_fd < 0){
        perror("Unable to accept socket");
        syslog(LOG_ERR,"Unable to accept socket");
        return 1;
        continue;
    }

    inet_ntop(connect_addr.ss_family, get_in_addr((struct sockaddr *) &connect_addr), s, sizeof s);
    printf("Accepted connection from %s\n",s);
    syslog(LOG_INFO,"Accepted connection from %s\n",s);

    if (!fork()){
        close(sockfd);
        if (send(new_fd, "connection accepted", 13, 0) == -1){
            perror("failed to send");
        }
        close(new_fd);
        perror("Unable to send data to socket");
        syslog(LOG_ERR,"Unable to send data to socket");

    }

  }

    return 0;
}