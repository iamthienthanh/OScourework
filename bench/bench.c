#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
enum { NS_PER_SECOND = 1000000000 };

void crash(void) {
    perror("Usage: bench [OPTION]... [COMMAND [ARG]...]\n");
    exit(EXIT_FAILURE);
}
void opt_error(char c) {
    fprintf(stderr,"bench: invalid argument type -- %c\n", c);
    crash();
}
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}
struct timespec timeadd(struct timespec t1, struct timespec t2) {
    struct timespec sum;
    sum.tv_sec = t1.tv_sec + t2.tv_sec;
    sum.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    if (sum.tv_nsec > NS_PER_SECOND) {
        sum.tv_nsec -= NS_PER_SECOND;
        sum.tv_sec++;
    }
    return sum;
}
struct timespec divided(struct timespec t, int d) {
    struct timespec result;
    size_t time = (int)((t.tv_sec * NS_PER_SECOND + t.tv_nsec)/d);
    result.tv_sec = (size_t)(time/NS_PER_SECOND);
    result.tv_nsec = (time % NS_PER_SECOND);
    return result;
}
int main(int argc, char *argv[]) {
    // Parse the command line arguments
    size_t w = 0;
    size_t d = 5;
    int opt;
    while ((opt = getopt(argc, argv, ":w:d:")) != -1) {
        char *p;
        switch (opt) {
        case 'w':
            w = strtol(optarg, &p, 10);
            if (w == '\0') opt_error(optopt);
            break;
        case 'd':
            d = strtol(optarg, &p, 10);
            if (d == '\0') opt_error(optopt);
            break;
        case ':':
            fprintf(stderr,"bench: option requires an argument -- %c\n", optopt);
            crash();
            break;
        case '?':
            fprintf(stderr,"bench: invalid option -- %c\n", optopt);
            crash();
            break;
        } 
    }
    // Get the command and command arguments
    const int optcount = optind;
    char *arg[argc - optcount + 1];
    char **p = arg;
    for (; optind < argc; optind++) {
        p[optind-optcount] = argv[optind];
    }
    if (!*arg[0]) {
        perror("bench: command requires");
        crash();
    }
    p[optind - optcount] = NULL;
    // ----Execute the command-------

    // Initialize variables
    int total_runs = 0;
    int runs = 0;
    int fails = 0;
    pid_t pid;
    int status;
    // Time variable
    struct timespec start, end, delta;
    struct timespec sum;
    struct timespec min, max;
    min.tv_sec = 0;
    min.tv_nsec = 0;
    max.tv_sec = 0;
    max.tv_nsec = 0;
    sum.tv_nsec = 0;
    sum.tv_sec = 0;
    // -- Execute loop ----
    while (sum.tv_sec < d) {
        if ((pid = fork()) < 0) {
            perror("bench: Error creating process");
        }
        if (pid != 0) { 
            clock_gettime(CLOCK_MONOTONIC, &start);
            waitpid(pid, &status, 0);
            clock_gettime(CLOCK_MONOTONIC, &end);
            if (status == 0 && total_runs > w) runs++;
            if (status != 0 && total_runs > w) fails++;
        } else {
            if (execvp((const char*)arg[0], arg) < 0) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        if (total_runs > w) {
            sub_timespec(start, end, &delta);
            if (min.tv_sec == 0 && min.tv_nsec == 0) {
                min.tv_nsec = delta.tv_nsec;
                min.tv_sec = delta.tv_sec;
            }
            if (delta.tv_sec >= max.tv_sec) {
                if (delta.tv_sec == max.tv_sec && delta.tv_nsec >= max.tv_nsec) {
                    max.tv_sec = delta.tv_sec;
                    max.tv_nsec = delta.tv_nsec;
                } 
                if (delta.tv_sec > max.tv_sec) {
                    max.tv_sec = delta.tv_sec;
                    max.tv_nsec = delta.tv_nsec;
                }
            }
            if (delta.tv_sec <= min.tv_sec) {
                if (delta.tv_sec == min.tv_sec && delta.tv_nsec < min.tv_nsec) {
                    min.tv_sec = delta.tv_sec;
                    min.tv_nsec = delta.tv_nsec;
                } 
                if (delta.tv_sec < min.tv_sec) {
                    min.tv_sec = delta.tv_sec;
                    min.tv_nsec = delta.tv_nsec;
                }
            }
            sum = timeadd(sum, delta);
        }
        total_runs ++;
    }        
        struct timespec avg = divided(sum, runs);
        
        // ------ Display results------------------------

        printf("Min: %d.%.9ld seconds    Warmups: %zu\n", (int)min.tv_sec, min.tv_nsec, w);
        printf("Avg: %d.%.9ld seconds    Runs:  %d\n", (int)avg.tv_sec, avg.tv_nsec, runs);
        printf("Max: %d.%.9ld seconds    Fails: %d\n", (int)max.tv_sec, max.tv_nsec, fails);
        printf("Total: %d.%.9ld seconds\n", (int)sum.tv_sec, sum.tv_nsec);

    return EXIT_SUCCESS;
}