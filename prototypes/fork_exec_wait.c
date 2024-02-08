#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>


int my_system(const char *cmd){
    int status;
    pid_t pid;

    pid = fork();
    if (pid == -1){
        perror("fork");
        return -1;
    } else if (pid == 0){
        const char *argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = cmd;
        argv[3] = NULL;
        execv('/bin/sh', argv);

        exit (-1);
    }

    if (wait(&status) == -1){
        return -1;
    }else if (WIFEXITED (status)){
        return WEXITSTATUS (status);
    }
    
    return -1;
        
}


int main(int argc, char *argv[]){

    const char *cmd = "/bin/ls";

    my_system(cmd);

}