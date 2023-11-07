#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>

#define DRIVER 1
#define TRAVELER 0
#define MAX_NUM_PEOPLE 100

// -------------------------------------------------------------
// Mutexes and condition variables
pthread_mutex_t mutex;
pthread_mutex_t mutex1;
pthread_cond_t traveler;
pthread_cond_t driver;
// Global variables
size_t waiting_traveler = 0;
size_t waiting_driver = 0;
struct person* STAND[MAX_NUM_PEOPLE];
size_t stand_nums = 0;
// Person struct
struct person {
    int id;
    int type;
};
// -------------------------------------------------------------

//  --------- Utility functions --------------------------------
// Initialize mutxes and condition variables
void initialize_locks() {
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Init lock failed");
    }
    if (pthread_cond_init(&traveler, NULL) != 0) {
        perror("Init CV failed");
    }
    if (pthread_cond_init(&driver, NULL) != 0) {
        perror("Init CV failed");
    }
}
// Get random time to sleep
float random_time(int min, int max) {
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}
// Stand operations
void stand_queue(struct person* person) {
    stand_nums++;
    if (stand_nums != 1) {
        for (int i = stand_nums - 1; i >= 1; i--) {
            STAND[i] = STAND[i-1];
        }
    }
    STAND[0] = person;
}
void stand_dequeue() {
    for (int i = 0; i < stand_nums; i++) {
        STAND[i] = STAND[i+1];
    }
    STAND[stand_nums] = NULL;
    stand_nums--;
}
void print_stand_queue() {
    printf("taxi:   [");
    for (int i = 0; i < stand_nums; i++) {
        switch (STAND[i]->type)
        {
        case DRIVER:
            printf("d");
            break;
        case TRAVELER:
            printf("t");
            break;
        }
        printf("%d", STAND[i]->id);
        if (i != stand_nums - 1) {
            printf(",");
        }
    }
    printf("]");
    // Balance the third column of the trace
    if (stand_nums == 0) {
        for (int i = 0; i<30; i++) {
            printf(" ");
        }
    } else {
        for (int i = 0; i < 30 - stand_nums*2 - stand_nums + 1; i++) {
            printf(" ");
        }
    }
}
// Tracing which travelers are waiting to be picked
void print_waiting_traveler() {
    for (int i = 0; i < stand_nums; i++) {
        if (STAND[i]->type == TRAVELER) {
            printf("t%d ", STAND[i]->id);
        }
    }
    printf("\n");
}
// Main synchronization problem
void stand_entering(struct person *p) {
    print_stand_queue();
    if (p->type == TRAVELER)
        printf("t%d entering\n", p->id);
    else
        printf("d%d entering\n", p->id);
    stand_queue(p);
}
void stand_waiting(struct person *p) {
    print_stand_queue();
    if (p->type == TRAVELER) {
        printf("t%d waiting ...\n", p->id);
        pthread_cond_wait(&driver, &mutex);
    }
    else {
        printf("d%d waiting ... \n", p->id);
        pthread_cond_wait(&traveler, &mutex);
    }
}
void stand_leaving(struct person *p) {
    print_stand_queue();
    if (p->type == TRAVELER)
        printf("t%d leaving\n", p->id);
    else
        printf("d%d leaving\n", p->id);
    stand_dequeue();
}
void pick_driver(struct person *p) {
    print_stand_queue();
    printf("d%d leaving\n", p->id);
    STAND[stand_nums - 1] = NULL;
    stand_nums--;
}
void travler_waking_up(struct person *p) {
    print_stand_queue();
    printf("...t%d waking up\n", p->id);
}
void picking_travelers(struct person *p) {
    print_stand_queue();
    printf("d%d picking traveler ", p->id);
    print_waiting_traveler();
    pthread_cond_broadcast(&driver);
}
// ----------------------------------------------------------
void stand_visit_traveler(struct person *p) {
    pthread_mutex_lock(&mutex);
    stand_entering(p);
    if (waiting_driver == 0) {
        waiting_traveler++;
        stand_waiting(p);
        travler_waking_up(p);
        stand_leaving(p);
        waiting_traveler--;
        pthread_mutex_unlock(&mutex);
    } else {
        pthread_cond_signal(&traveler);
        stand_leaving(p);
        pthread_mutex_unlock(&mutex);
    }
}

void stand_visit_driver(struct person *p) {
    pthread_mutex_lock(&mutex);
    stand_entering(p);
    if (waiting_traveler == 0) {
        waiting_driver++;
        stand_waiting(p);
        pick_driver(p);
        waiting_driver--;
        pthread_mutex_unlock(&mutex);
    } else {
        picking_travelers(p);
        stand_leaving(p);
        pthread_mutex_unlock(&mutex);
    }
}
void *person_life(void *p) {
    struct person *person = (struct person *)p;
    while (1) {
        switch (person->type) {
        case DRIVER:
            stand_visit_driver(person);
            break;
        case TRAVELER:
            stand_visit_traveler(person);
            break;
        }
        sleep(random_time(5, 10));
    }
}
// ----------------------------------------------------------------
// Handle errors
void crash(void) {
    perror("Usage: taxi [OPTION]");
    exit(EXIT_FAILURE);
}
void opt_error(char *c) {
    fprintf(stderr, "taxi: invalid argument type -- %s\n", c);
    crash();
}

// ---- Main function ----------
int main (int argc, char *argv[]) {
    srand(time(NULL));
    size_t t = 2, d = 1;
    // Parse the command line arguments
    int opt;
    while ((opt = getopt(argc, argv, ":t:d:")) != -1)
    {
        switch (opt)
        {
        case 't':
            t = strtol(optarg, NULL, 10);
            if (t == '\0') opt_error(optarg);
            break;
        case 'd':
            d = strtol(optarg, NULL, 10);
            if (d == '\0') opt_error(optarg);
            break;
        case ':':
            fprintf(stderr, "taxi: option requires an argument -- %c\n", optopt);
            crash();
            break;
        case '?':
            fprintf(stderr, "taxi: invalid option -- %c\n", optopt);
            crash();
            break;
        default:
            break;
        }
    }
    // Main loop 
    initialize_locks();
    pthread_t traveler_id[t];
    pthread_t driver_id[t];
    struct person* travelers[t];
    struct person* drivers[d];

    // Create threads for each person
    for (int i = 0; i < t; i++) {
        travelers[i] = malloc(sizeof(struct person));
        travelers[i]->id = i;
        travelers[i]->type = TRAVELER;
        int ret = pthread_create(&traveler_id[i], NULL, person_life, (void*) travelers[i]);
        if (ret != 0) {
            fprintf(stderr,"Error: pthread_create() failed\n");
            return (EXIT_FAILURE);
        }
    }
    for (int i = 0; i < d; i++) {
        drivers[i] = malloc(sizeof(struct person));
        drivers[i]->id = i;
        drivers[i]->type = DRIVER;
        int ret = pthread_create(&driver_id[i], NULL, person_life, (void*) drivers[i]);
        if (ret != 0) {
            fprintf(stderr,"Error: pthread_create() failed\n");
            return (EXIT_FAILURE);
        }
    }
    // Join all threads
    for (int i = 0; i < t; i++) {
        if (pthread_join(traveler_id[i], NULL) != 0) {
            fprintf(stderr, "Error: pthread_join() failed\n");
            return (EXIT_FAILURE);
        }
    }
    for (int i = 0; i < d; i++) {
        if (pthread_join(driver_id[i], NULL) != 0) {
            fprintf(stderr, "Error: pthread_join() failed\n");
            return (EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}