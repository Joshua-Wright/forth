// (c) Copyright 2016 Josh Wright

#include <string.h>
#include <stdio.h>
#include "util.h"
#include "eval.h"

static stack_t __stack[STACK_SIZE] = {0};
stack_t *stack = ((stack_t *) __stack) + STACK_SIZE - 1;
static word_t **__return_stack[RETURN_STACK_SIZE] = {0};
word_t ***return_stack = ((word_t ***) __return_stack) + RETURN_STACK_SIZE - 1;
word_t **prog_counter = 0;

void call() {
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


void default_interpreter() {
    // push return value
    return_stack--;
    return_stack[0] = prog_counter;

    // find code and set prog counter
    custom_word_t *custom_word = (custom_word_t *) *prog_counter;
    prog_counter = &custom_word->code[0];

    // run code
    do {
        call();
    } while (prog_counter[0] != NULL);

    // pop program counter
    prog_counter = return_stack[0];
    return_stack++;
}

void init_stdlib() {
    const char *lib =
            ": inc 1 + ; "
                    ": dec 1 - ; "
                    "";
    eval_str(lib);
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

DECLARE_WORD("*", mul, &word_sub) {
    stack[1] = stack[0] * stack[1];
    stack++;
}

DECLARE_WORD("/", div, &word_mul) {
    stack[1] = stack[1] / stack[0];
    stack++;
}

DECLARE_WORD("dup", dup, &word_div) {
    stack--;
    stack[0] = stack[1];
}

DECLARE_WORD("dup2", dup2, &word_dup) {
    stack -= 2;
    stack[0] = stack[2];
    stack[1] = stack[3];
}

DECLARE_WORD("swap", swap, &word_dup2) {
    stack_t tmp = stack[0];
    stack[0] = stack[1];
    stack[1] = tmp;
}

DECLARE_WORD(".", print_signed_int, &word_swap) {
    printf("%li\n", (long) stack[0]);
    stack++;
}

DECLARE_WORD("lit", literal, &word_print_signed_int) {
    stack--;
    prog_counter++;
    stack[0] = (stack_t) prog_counter[0];
}

DECLARE_WORD("!", bang, &word_literal) {
    int64_t *addr = (int64_t *) stack[0];
    int64_t x = stack[1];
    *addr = x;
    stack += 2;
}

DECLARE_WORD("over", over, &word_bang) {
    stack--;
    stack[0] = stack[2];
}

DECLARE_WORD("rot", rot, &word_over) {
    stack_t s0 = stack[0];
    stack_t s1 = stack[1];
    stack_t s2 = stack[2];
    stack[0] = s1;
    stack[1] = s2;
    stack[2] = s0;
}

word_t *top_word = &word_rot;
