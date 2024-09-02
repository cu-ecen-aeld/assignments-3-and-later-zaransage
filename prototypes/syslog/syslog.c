#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

/*
Bust the rust on some of my c programming basics
*/

int main( int argc, char *argv[]){

  openlog(NULL,0,LOG_USER);

  if(argc == 1) {
    printf("You need two arguments but you only gave: None\n");
    syslog(LOG_ERR,"Invalid number of arguments: No arguments given");
    exit(1);
  }
  if(argc < 3) {
    printf("You need two arguments but you only gave: %s\n", argv[1]);
    syslog(LOG_ERR,"Invalid number of arguments: %s", argv[1]);
    exit(1);
  }
  if(argc > 3) {
    printf("You need only two arguments but you gave: %s %s %s\n", argv[1], argv[2], argv[3]);
    syslog(LOG_ERR, "You need only two arguments but you gave: %s %s %s\n", argv[1], argv[2], argv[3]);
    exit(1);
  }

}