#include <stdio.h>
#include <time.h>

// https://datatracker.ietf.org/doc/html/rfc2822

int main() {
   
    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

    char buffer[80];
    // Mon 02 Sep 2024 08:53:12 +0000
    strftime(buffer, sizeof(buffer), "%a %d %b %y %r %z", local);

    printf("%s\n", buffer);

    return 0;
}