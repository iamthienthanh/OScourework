#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <pthread.h>
// is Perfect function
static int is_perfect(uint64_t num) {
    uint64_t i, sum;
    if (num < 2) return 0;
    for (i = 2, sum = 1; i*i <= num; i++) {
        if (num % i == 0) {
            sum += (i*i == num) ? i : i + num / i;
        }
    }
    return (sum == num);
}
// handle crash cases
void crash(void) {
    perror("Usage: perfect [OPTION]");
    exit(EXIT_FAILURE);
}
// handle wrong type arguments
void opt_error(char* c) {
    fprintf(stderr,"perfect: invalid argument type -- %s\n", c);
    crash();
}
// Argument to be passed to thread function
struct range {
    size_t thid, low, high;
};
// Function to call in threads
void *perfect(void* arg) {
    struct range* r = (struct range*)arg;
    for (int i = r->low; i < r->high; i++) {
        if (is_perfect(i)) printf("%d\n", i);
    }
    pthread_exit(NULL);
}
int main(int argc, char **argv) {
    // ------ Get the option arguments --------------------------------
    int opt;
    int v = 0;
    size_t t = 1;
    size_t s = 1;
    size_t e = 10000;
    char *p;
    while ((opt = getopt(argc, argv, ":s:e:t:v")) != -1) {
        switch (opt) {
        case 's':
            s = strtol(optarg, &p, 10);
            if (s == '\0') opt_error(optarg);
            break;
        case 'e':
            e = strtol(optarg, &p, 10);
            if (e == '\0') opt_error(optarg);
            break;
        case 't':
            t = strtol(optarg, &p, 10);
            if (t == '\0') opt_error(optarg);
            break;
        case 'v':
            v = 1;
            break;
        case ':':
            fprintf(stderr,"perfect: option requires an argument -- %c\n", optopt);
            crash();
            break;
        case '?':
            fprintf(stderr,"perfect: invalid option -- %c\n", optopt);
            crash();
            break;
        default:
            break;
        }
    }

    // --- Set up intervals for searching-----
    
    int step = (e-s)/t + 1;
    pthread_t thid[t];
    struct range range[t];
    for (int i = 0; i < t; i++) {
            range[i].thid = i;
            range[i].low = s + step * i;
            range[i].high = s + step *(i + 1) - 1;
    }
    // ------------- Search ----------------------------
    for (size_t id = 0; id < t; id++) {
        if (v) fprintf(stderr,"perfect: t%zu searching [%zu, %zu]: \n", range[id].thid, range[id].low, range[id].high);
        int ret = pthread_create(&thid[id], NULL, perfect,(void *) &range[id]);
        if (ret != 0) {
            fprintf(stderr,"Error: pthread_create() failed\n");
            return EXIT_FAILURE;
        }
    }
    // Wait for threads to finished
    for (size_t id = 0; id < t; id++) {
        if (pthread_join(thid[id], NULL) != 0) {
            fprintf(stderr, "Error: pthread_join() failed\n");
            return EXIT_FAILURE;
        } else if (v) fprintf(stderr, "perfect: t%zu finishing\n", range[id].thid);
    }
    return EXIT_SUCCESS;
}