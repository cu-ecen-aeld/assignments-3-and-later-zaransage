#include <stdio.h>
#include <stdlib.h>


int main(){

  const char *command = "ls";
  int my_command = system(command);
    
    switch (my_command) {
        case -1:
            perror("Error");
            return false();
        case 0:
            return true();
        default:
            exit(EXIT_SUCCESS);
    }

}