#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*
Outside Reference: https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
*/


/* 
One difference from the write.sh instructions in Assignment 1:  
You do not need to make your "writer" utility create directories which do not exist.  
You can assume the directory is created by the caller.

Setup syslog logging for your utility using the LOG_USER facility.

Use the syslog capability to write a message “Writing <string> to <file>” where <string> 
is the text string written to file (second argument) and <file> is the file created by the script.  
This should be written with LOG_DEBUG level.

Use the syslog capability to log any unexpected errors with LOG_ERR level. 
 
 */


int file_create(char *file, char *message){
  const int err = errno;

  int fd;
  const char *buf;
  ssize_t nr;

  fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);

  if (fd == -1){
    printf("File does not exist\n: %d", fd);
    syslog(LOG_ERR, "File does not exist: %d: %s\n", fd, strerror(err));
    exit(1);
  } 

  buf = message;
  nr = write(fd, buf, strlen(buf));
  syslog(LOG_DEBUG, "Writing %s to %s: %s", message, file, strerror(err));
}


int main(int argc, char *argv[]){

  const int err = errno;
  openlog(NULL,0,LOG_USER);

  if(argc == 1) {
    printf("You have not specified 'writefile' argument\n");
    syslog(LOG_ERR,"Invalid number of arguments: You have not specified a \'writefile\' argument: %s", strerror(err));
    exit(1);
  }
  if(argc < 3) {
    printf("You have not specified writestr argument\n");
    syslog(LOG_ERR,"Invalid number of arguments: You have not specified a \'writestr\' argument: %s", strerror(err));
    exit(1);
  }
  if(argc > 3) {
    printf("You need only two arguments but you gave: %s %s %s %s\n", argv[1], argv[2], argv[3], strerror(err));
    syslog(LOG_ERR, "You need only two arguments but you gave: %s %s %s %s\n", argv[1], argv[2], argv[3], strerror(err));
    exit(1);
  }

  file_create(argv[1], argv[2]);

}