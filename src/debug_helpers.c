// (c) Copyright 2016 Josh Wright
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "debug_helpers.h"

char *safe_word_name(word_t *word) {
    word_t *cur = top_word;
    while (cur != NULL) {
        if (cur == word) {
            return cur->name;
        }
        cur = cur->prev;
    }
    return NULL;
}

void pretty_print_custom_word(custom_word_t *custom_word) {
    printf("%p: %s\n", custom_word, custom_word->word.name);

    for (int i = 0; i < custom_word->code_size; ++i) {
        char *word_name = safe_word_name(custom_word->code[i]);
        if (word_name != NULL) {
            printf("%12p %6i %6x: %12p %12lli %s\n", &custom_word->code[i], i, i, custom_word->code[i],
                   (long long int) custom_word->code[i],
                   safe_word_name(custom_word->code[i]));
        } else {
            printf("%12p %6i %6x: %12p %12lli\n", &custom_word->code[i], i, i, custom_word->code[i], (long long int) custom_word->code[i]);
        }
    }

}

void pretty_print_custom_word_by_name(char *name) {
    word_t *cur = top_word;
    while (cur != NULL) {
        if (strcmp(name, cur->name) == 0) {
            pretty_print_custom_word((custom_word_t *) cur);
            return;
        }
        cur = cur->prev;
    }
    printf("word not found: %s\n", name);
}


