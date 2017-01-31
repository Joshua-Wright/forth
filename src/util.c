// (c) Copyright 2016 Josh Wright

#include <string.h>
#include <stdio.h>
#include "util.h"

static stack_t __stack[STACK_SIZE] = {0};
stack_t *stack = ((stack_t *) __stack) + STACK_SIZE - 1;
static word_t **__return_stack[RETURN_STACK_SIZE] = {0};
word_t ***return_stack = ((word_t ***) __return_stack) + RETURN_STACK_SIZE - 1;
word_t **prog_counter = 0;

void call() {
    if (*prog_counter == NULL) {
        // return if we reached the end of a block
        prog_counter = return_stack[0];
        return_stack++;
    }
    word_t *word = *prog_counter;
    word->interpreter();
    prog_counter++;
}

word_t *lookup_word(char *name) {
    word_t *cur = top_word;
    while (cur != NULL) {
        if (strcmp(name, cur->name) == 0) {
            return cur;
        }
        cur = cur->prev;
    }
    return NULL;
}


////////////////////////////////////////////////////////////

DECLARE_WORD("+", add, NULL) {
    stack_t res = stack[0] + stack[1];
    stack++;
    stack[0] = res;
}

DECLARE_WORD("-", sub, &word_add) {
    stack_t res = stack[0] - stack[1];
    stack++;
    stack[0] = res;
}

DECLARE_WORD(".", print_signed_int, &word_sub) {
    printf("%i\n", stack[0]);
    stack++;
}

DECLARE_WORD("lit", literal, &word_print_signed_int) {
    stack--;
    prog_counter++;
    stack[0] = (stack_t) prog_counter[0];
}

word_t *top_word = &word_literal;
