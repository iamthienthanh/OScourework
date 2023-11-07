#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#define MAX_RESPONSE_SIZE 4096

char *fetch(char *url)  {
    // Initialize pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("quiz: error creating pipe");
        return NULL;
    }
    // Create child process
    pid_t pid = fork();
    if (pid <0) {
        perror("quiz: error creating child process");
        return NULL;
    }
    if (pid == 0) {
        // close unused read end
        close(fd[0]);
        // redirect stdout to pipe
        dup2(fd[1], STDOUT_FILENO);
        // close write end
        close(fd[1]);              
        char *args[] = {"curl","-s", url, NULL};
        if (execvp("curl", args) < 0) {
            exit(EXIT_FAILURE);
        };
        exit(EXIT_SUCCESS);
    }
    else {  // parent process
        // close unused write end
        close(fd[1]); 
        // Read result from child process
        char response[MAX_RESPONSE_SIZE] = {0};
        if (read(fd[0], response, MAX_RESPONSE_SIZE) == -1) {
            perror("quiz: error reading response");
            return NULL;
        }
        // close read end
        close(fd[0]); 
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return strdup(response);
        }
        else {
            perror("quiz: can not fetch quiz");
            return NULL;
        }
    }
}