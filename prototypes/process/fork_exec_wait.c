#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/wait.h>



int my_system(const char *cmd){
    int status;
    pid_t pid;

    pid = fork();
    if (pid == -1){
        perror("fork");
        return -1;
    } else if (pid == 0){
        char *argv[] = {"/bin/ls", "/tmp/", NULL};
        execv(argv[0], argv);
        exit (-1);
    }

    if (wait(&status) == -1){
        return -1;
    }else if (WIFEXITED (status)){
        return WEXITSTATUS (status);
    }
    
    return -1;
        
}


int main(){

    int status;
    pid_t pid;

    pid = fork();
    if (pid == -1){
        perror("fork");
        return -1;
    } else if (pid == 0){
        char *argv[] = {"/bin/ls", "/tmp/", NULL};
        execv(argv[0], argv);
        exit (-1);
    }

    if (wait(&status) == -1){
        return -1;
    }else if (WIFEXITED (status)){
        return WEXITSTATUS (status);
    }
    
    return -1;



}