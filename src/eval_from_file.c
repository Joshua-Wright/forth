// (c) Copyright 2016 Josh Wright

#include "eval_from_file.h"

void eval_from_file(FILE *fp) {
    char namebuf[256];
    while (fscanf(fp, "%255s", namebuf) != EOF) {
        if (namebuf[0] == ':' && namebuf[1] == '\0') {
            // TODO user-defined functions
        } else {
            word_t *word = lookup_word(namebuf);
            if (word == NULL) {
                stack--;
                stack[0] = atoi(namebuf);
            } else {
                word_t *mini_code[] = {word, NULL};
                prog_counter = &mini_code[0];
                call();
            }
        }
    }

}
