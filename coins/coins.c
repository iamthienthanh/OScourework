#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>
#include <time.h>

#define NUM_COINS 20

char COINS[NUM_COINS];
pthread_mutex_t global_lock;
pthread_mutex_t coins_lock[NUM_COINS];
size_t p = 100;
size_t n = 10000;
// Initialize locks 
void initialize_locks() {
    if (pthread_mutex_init(&global_lock, NULL) != 0) {
        perror("Init lock failed");
    }
    for (size_t i = 0; i < NUM_COINS; i++) {
        if (pthread_mutex_init(&coins_lock[i], NULL) != 0) {
            perror("Init lock failed");
        }
    }
}
// Handle errors
void crash(void) {
    perror("Usage: coins [OPTION]");
    exit(EXIT_FAILURE);
}
void opt_error(char *c) {
    fprintf(stderr, "coins: invalid argument type -- %s\n", c);
    crash();
}
// Some utility functions
void flip(char* coin) {
    int r = rand() % 2;
    *coin = r == 0 ? '0' : 'X';
}
void initialize_coins() {
    for (int i = 0; i < NUM_COINS; i++) {
        flip(&COINS[i]);
    }
}
void flip_coins(int separate_lock) {
    for (int i = 0; i < NUM_COINS; i++) {
        if (separate_lock) pthread_mutex_lock(&coins_lock[i]);
        flip(&COINS[i]);
        if (separate_lock) pthread_mutex_unlock(&coins_lock[i]);
    }
}
// Run threads function
void run_threads(size_t p, void *(*proc)(void *)) {
    pthread_t thid[p];
    for (size_t i = 0; i < p; i++) {
        int ret = pthread_create(&thid[i], NULL, proc, (void *) &n);
        if (ret != 0) {
            fprintf(stderr,"Error: pthread_create() failed\n");
            exit(EXIT_FAILURE);
        }
    }
    for (size_t i = 0; i < p; i++) {
        if (pthread_join(thid[i], NULL) != 0) {
            fprintf(stderr, "Error: pthread_join() failed\n");
            exit(EXIT_FAILURE);
        }
    }
}
// Timeit function
static double timeit(int n, void *(*proc)(void *)) {
    clock_t t1, t2;
    t1 = clock();
    run_threads(n, proc);
    t2 = clock();
    return ((double)t2 - (double)t1) / CLOCKS_PER_SEC * 1000;
}
// Strategies for flipping coins
void* strategy_1(void* arg) {
    size_t n = *(size_t*)arg;
    pthread_mutex_lock(&global_lock);
    for (int i = 0; i < n; i++) {
        flip_coins(0);
    }
    pthread_mutex_unlock(&global_lock);
    return NULL;
}
void* strategy_2(void* arg) {
    size_t n = *(size_t*)arg;
    for (int i = 0; i < n; i++) {
        pthread_mutex_lock(&global_lock);
        flip_coins(0);
        pthread_mutex_unlock(&global_lock);
    }
    return NULL;
}
void* strategy_3(void* arg) {
    size_t n = *(size_t*)arg;
    for (int i = 0; i < n; i++) {
        flip_coins(1);
    }
}
int main(int argc, char **argv) {
    // Parse the command line arguments
    int opt;
    while ((opt = getopt(argc, argv, ":p:n:")) != -1)
    {
        switch (opt) {
        case 'p':
            p = strtol(optarg, NULL, 10);
            if (p == '\0')
                opt_error(optarg);
            break;
        case 'n':
            n = strtol(optarg, NULL, 10);
            if (n == '\0')
                opt_error(optarg);
            break;
        case ':':
            fprintf(stderr, "coins: option requires an argument -- %c\n", optopt);
            crash();
            break;
        case '?':
            fprintf(stderr, "coins: invalid option -- %c\n", optopt);
            crash();
            break;
        default:
            break;
        }
    }
    srand(time(NULL));
    double exe_time;
    initialize_locks();
    // Strategy 1
    initialize_coins();
    printf("coins: %s (start - global lock)\n", COINS);
    exe_time = timeit(p, strategy_1);
    printf("coins: %s (end - global lock)\n", COINS);
    printf("%zu threads x %zu flips: %.3f ms\n",p,n, exe_time);
    // Strategy 2
    initialize_coins();
    printf("coins: %s (start - iteration lock)\n", COINS);
    exe_time = timeit(p, strategy_2);
    printf("coins: %s (end - iteration lock)\n", COINS);
    printf("%zu threads x %zu flips: %.3f ms\n",p,n, exe_time);
    // Strategy 3
    initialize_coins();
    printf("coins: %s (start - coin lock)\n", COINS);
    exe_time = timeit(p, strategy_3);
    printf("coins: %s (end - coin lock)\n", COINS);
    printf("%zu threads x %zu flips: %.3f ms\n",p,n, exe_time);

    return EXIT_SUCCESS;
}