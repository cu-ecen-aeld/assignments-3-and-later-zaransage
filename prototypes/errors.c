#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(){
    const int err = errno;

    int a = 1;
    int b = 2;
  
    if (a == b){
        printf("My error number is: %d\n", err);
        printf("My error message is: %s\n", strerror (err));
        printf("Equal\n");
    } else {
        printf("Different\n");
        printf("My error message is: %s\n", strerror (err));
        printf("My error number is: %d\n", err);
    }

}