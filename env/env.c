#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;
int check_invalid_name(char* name) {
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '=') {
            fprintf(stderr, "The value for name must not include the '=' character.\n");
            return 1;
        }
    }
    return 0;
}
void crash() {
    fprintf(stderr, "Usage: env [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char*argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, ":0iP:S:u:v")) != -1) {
        switch(opt) {
            case 'u':
                if (check_invalid_name(optarg) == 1) crash();
                break;
            case ':':
                fprintf(stderr,"env: option requires an argumentt -- %c\n", optopt);
                crash();
                break;
            case '?':
                fprintf(stderr,"env: illegal option -- %c\n", optopt);
                crash();
                break;
        }
    }
    if (fork() == 0) {
        int status_code = execvp("env", argv);
        if (status_code == -1) {
            fprintf(stderr, "Terminate incorrectly\n");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}