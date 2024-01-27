#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>


int file_create(char *file){

  int fd;
  
  fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);

  if (fd == -1){
    printf("File does not exist: %d", fd);
    syslog(LOG_ERR, "File does not exist: %d", fd);
    exit(1);
  } 
}


int main(){

  char *path = "/tmp/my_test_5.txt";
  file_create(path);
  

}