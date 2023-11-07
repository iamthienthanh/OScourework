#include <stdio.h>
#include <stdlib.h>
#include "quiz.h"
#include <signal.h>

int CURRENT_QUESTION = 1;
int CURRENT_SCORE = 0;
int CURRENT_MAX_SCORE = 0;

void interrupt(int signum) {
    printf("\nYour current score is %d/%d points.\n\nThanks for playing today.\nYour final score is %d/%d points.\n", CURRENT_SCORE, CURRENT_MAX_SCORE, CURRENT_SCORE, CURRENT_MAX_SCORE);
    exit(signum);
}

int main(int argc, char **argv) {
    // Handle user's interrupt signals
    signal(SIGINT, interrupt);
    // Game introduction
    puts("Answer multiple choice questions about computer science.\nYou score points for each correctly answered question.\nIf you need multiple attempts to answer a question, the\npoints you score for a correct answer go down.\n");
    // Game loop
    while (1) {
        quiz_t quiz;
        quiz.n = CURRENT_QUESTION;
        quiz.max = CURRENT_MAX_SCORE;
        quiz.score = CURRENT_SCORE;
        if (play(&quiz) !=0) continue; // if there was an error, continue to the next question
        CURRENT_QUESTION++;
    }
    return 0;
}