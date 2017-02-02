// (c) Copyright 2016 Josh Wright

#include <string.h>
#include "eval.h"

void read_function(FILE *fp);

void eval_file(FILE *fp) {
    char namebuf[256];
    while (fscanf(fp, "%255s", namebuf) != EOF) {
        if (namebuf[0] == ':' && namebuf[1] == '\0') {
            read_function(fp);
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

stack_t eval_str(const char *str) {
    FILE *fp = fmemopen((void *) str, strlen(str), "r");
    eval_file(fp);
    return stack[0];
}

void read_function(FILE *fp) {
    // allow 255 letters per word, 255 words per function
    char namebuf[256] = {0};

    // initialize first parts of the data
    /* size_t alloc_size = sizeof(word_t) + sizeof(word_t*)*256; */
    size_t alloc_size = sizeof(custom_word_t);
    custom_word_t *word_func = (custom_word_t *) malloc(alloc_size);
    memset(word_func, 0, alloc_size);
    word_func->word.interpreter = default_interpreter;
    word_func->word.prev = top_word;
    top_word = &word_func->word;
    size_t word_idx = 0;

    // read function name
    fscanf(fp, "%255s", namebuf);
    word_func->word.name = malloc(strlen(namebuf) + 1);
    strcpy(word_func->word.name, namebuf);

    // read in all the word of the function contents
    fscanf(fp, "%255s", namebuf);
    while (!(namebuf[0] == ';' && namebuf[1] == '\0')) {
        word_t *word = lookup_word(namebuf);
        if (word == NULL) { // numeric literal
            word_func->code[word_idx] = &word_literal;
            word_idx++;
            word_func->code[word_idx] = (word_t *) atol(namebuf);
            word_idx++;

        } else { // function that we already have
            word_func->code[word_idx] = word;
            word_idx++;
            if (word->post_compile_hook != NULL) {// compile-time special word
                word->post_compile_hook(word_func, &word_idx);
            }
        }
        fscanf(fp, "%255s", namebuf);
    }
}
