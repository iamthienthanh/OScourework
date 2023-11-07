#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <jansson.h>
#include <string.h>
#include "quiz.h"

int parse(quiz_t *quiz, char *msg) {
    // Load the response JSON
    json_error_t err;
    json_t *json = json_loads(msg, 0, &err);
    if (!json) {
        fprintf(stderr,"quiz: error parsing JSON : on line %d : %s\n", err.line, err.text);
        return -1;
    }
    // Parse the response JSON to message
    json_t *result = json_array_get(json_object_get(json, "results"),0);
    quiz->question = strdup(json_string_value(json_object_get(result, "question")));
    quiz->answer = strdup(json_string_value(json_object_get(result, "correct_answer")));
    // Randomly arrange the correct answer
    srand(time(NULL));
    int random = rand() % 4;
    for (int i = 0; i < 4; i++) {\
        if (i == random) quiz->choices[i] = quiz->answer;
        else if (i < random) {
            quiz->choices[i] = strdup(json_string_value(json_array_get(json_object_get(result, "incorrect_answers"),i)));
        } else {
            quiz->choices[i] = strdup(json_string_value(json_array_get(json_object_get(result, "incorrect_answers"),i-1)));
        }
    }
    return EXIT_SUCCESS;
}