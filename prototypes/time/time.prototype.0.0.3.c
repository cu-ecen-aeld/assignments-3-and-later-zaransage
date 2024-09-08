#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>


void myTime() {
    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %d %b %y %r %z", local);

    FILE *fp = fopen("/tmp/time.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
    }

    fprintf(fp, "%s\n", buffer);
    fclose(fp);
}


int main() {

    myTime();

    return 0;
}