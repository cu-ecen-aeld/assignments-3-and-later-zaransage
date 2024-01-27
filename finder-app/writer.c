#include <stdio.h>
#include <syslog.h>
#include <errno.h>

/*
Outside Reference: https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
*/


/* 
One difference from the write.sh instructions in Assignment 1:  You do not need to make your "writer" utility create directories which do not exist.  You can assume the directory is created by the caller.

Setup syslog logging for your utility using the LOG_USER facility.

Use the syslog capability to write a message “Writing <string> to <file>” where <string> is the text string written to file (second argument) and <file> is the file created by the script.  This should be written with LOG_DEBUG level.

Use the syslog capability to log any unexpected errors with LOG_ERR level. 

 * */

/*
Check arguments 
*/

char argumentCheck(int argc, char *argv[]) {
  const int err = errno;

  if(argc < 2) {
    printf("You need two arguments but you gave: %s\n", argv[1]);
    syslog(LOG_ERR,"Invalid number of arguments: %d", argv[1]);
    exit(1);
  }
  if(argc > 2) {
    printf("You need only two arguments but you gave: %s\n", argv[1]);
    syslog(LOG_ERR,"Invalid number of arguments: %d", argv[1]);
    exit(1);
  }
    
}


int file_create(char file){
  const int err = errno;
  char fd = open(file, O_WRONLY | O_TRUNC | O_CREATE);

  if (fd == -1){
    printf("File does not exist: %s", fd, strerror (err));
    syslog(LOG_ERR, "File does not exist: %s", fd, strerror (err));
    exit(1);
  } else {
    fclose(fd);
  }
}


int main(int argc, char *argv[]){
    openlog(NULL,0,LOG_USER);
    argumentCheck(argc, *argv[0]);
    file_create(argv[2]);



}