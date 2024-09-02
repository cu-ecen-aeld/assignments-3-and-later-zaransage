#include <stdio.h>

/*
Bust the rust on some of my c programming basics
*/

int main( int argc, char *argv[]){

  if(argc < 3) {
    printf("You need two arguments but you only gave: %s\n", argv[1]);
  }
  if(argc > 3) {
    printf("You need only two arguments but you gave: %s %s %s\n", argv[1], argv[2], argv[3]);
  }

}