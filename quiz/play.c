#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "quiz.h"

#define SCORE_PER_ANSWER 8
extern int CURRENT_QUESTION;
extern int CURRENT_SCORE;
extern int CURRENT_MAX_SCORE;
extern void interrupt(int);
// Decode some html escape entities
void decode_html_entities(char *str) {
    char *p = str;
    char *q = str;
    while (*p) {
        if (*p == '&') {
            if (strncmp(p, "&amp;", 5) == 0) {
                *q++ = '&';
                p += 5;
            } else if (strncmp(p, "&lt;", 4) == 0) {
                *q++ = '<';
                p += 4;
            } else if (strncmp(p, "&gt;", 4) == 0) {
                *q++ = '>';
                p += 4;
            } else if (strncmp(p, "&quot;", 6) == 0) {
                *q++ = '"';
                p += 6;
            } else if (strncmp(p, "&#039;", 6) == 0) {
                *q++ = '\'';
                p += 6;
            } else if (strncmp(p, "&apos;", 6) == 0) {
                *q++ = '\'';
                p += 6;
            }
            else {
                *q++ = *p++;
            }
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
}

int play(quiz_t *quiz) {
    // Get the question and parse to quiz
    char *respone = fetch("https://opentdb.com/api.php?amount=1&category=18&type=multiple");
    if (!respone) return -1;
    decode_html_entities(respone);
    if (parse(quiz, respone) != 0) return -1;
    
    // Play the quiz
    puts(quiz->question);
    char c = 'a';
    for (int i = 0; i < 4; i++)
    {
        printf("[%c] %s\n",c+i,quiz->choices[i]);
    }
    puts("");

    // Get answer from user input
    int CURRENT_QUIZ_SCORE = SCORE_PER_ANSWER;
    char OPTION;
    int attempt = 3;
    while (attempt > 0)
    {
        printf("(%dpt) > ",CURRENT_QUIZ_SCORE);
        if (scanf(" %c", &OPTION) == EOF) {
            interrupt(0); // handle ^D end of transmit interruption
        }
        if (tolower(OPTION) < 97 || tolower(OPTION) > 100) puts("Please give a valid option");
        else if (strcmp(quiz->choices[(int)(tolower(OPTION) - 'a')], quiz->answer) != 0) {
            printf("Answer [%c] is wrong, try again.\n", OPTION);
            CURRENT_QUIZ_SCORE = (int)CURRENT_QUIZ_SCORE / 2;
            attempt--;
            if (attempt == 0) {
                CURRENT_MAX_SCORE += SCORE_PER_ANSWER;
                printf("\nThe correct answer is %s. Your current score is %d/%d points.\n\n", quiz->answer, CURRENT_SCORE, CURRENT_MAX_SCORE);
            }
        }
        else
        {
            CURRENT_SCORE += CURRENT_QUIZ_SCORE;
            CURRENT_MAX_SCORE += SCORE_PER_ANSWER;
            printf("Congratulation, answer [%c] is correct. Your current score is %d/%d points.\n\n", OPTION, CURRENT_SCORE, CURRENT_MAX_SCORE);
            break;
        }
    }

    return 0;
}