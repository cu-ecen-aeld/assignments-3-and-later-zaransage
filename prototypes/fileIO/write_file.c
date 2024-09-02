#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(){

    int fd;
    const char *buf;
    ssize_t nr;

    char *path = "/tmp/my_test_6.txt";
    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);

    buf = "Text content to write\n";

    nr = write(fd, buf, strlen(buf));

}