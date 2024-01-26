#include <stdio.h>
#include <syslog.h>
#include <errno.h>


/* 
One difference from the write.sh instructions in Assignment 1:  You do not need to make your "writer" utility create directories which do not exist.  You can assume the directory is created by the caller.

Setup syslog logging for your utility using the LOG_USER facility.

Use the syslog capability to write a message “Writing <string> to <file>” where <string> is the text string written to file (second argument) and <file> is the file created by the script.  This should be written with LOG_DEBUG level.

Use the syslog capability to log any unexpected errors with LOG_ERR level. 

 * */

/*
Check to see if the first argument is specified
*/

int argumentCheck() {
    openlog(NULL,0,LOG_USER)
    syslog(LOG_ERR,"Invalid number of arguments: %d", argc);
}

/*
Check to see if the second argument is specified
*/

/*
Create the file asked for
*/

int file_create(int file){

/* I might need to define the file as a memory object here*/

  const int err = errno;

  fd = fopen(file, O_WRONLY| O_TRUNC);

  if (fd == -1){
    syslog(LOG_ERR, "File does not exist: %s", fd, strerror (err));  
  } else {
    fclose(fd)
  }
}

/*
Set up Logger and syslog
Facilities and Priorities

*/

int main() {

}