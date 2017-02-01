// (c) Copyright 2016 Josh Wright
#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define STACK_SIZE 5120
#define RETURN_STACK_SIZE 256

typedef void(*interpreter_t)();
typedef int64_t stack_t;
typedef struct _word_t {
    struct _word_t *prev;
    char *name;
    interpreter_t interpreter;
} word_t;
typedef struct _custom_word_t {
    word_t word;
    // null-terminated array of words that make up the function
    word_t *code[256];
} custom_word_t;

/**
 * stack grows down, and this points to bottom of the stack.
 * e.g. stack[0] is top, stack[1] is second to top
 * push to stack by:
 * stack--;
 * *stack=thing;
 */
extern stack_t *stack;
/**
 * same stack semantics as above
 */
extern word_t ***return_stack;
extern word_t **prog_counter;

extern word_t *top_word;

/**
 * function call
 */
void call();

word_t *lookup_word(char *name);

void default_interpreter();

extern word_t word_literal;

#define DECLARE_WORD(name, identifier, prev_identifier) \
    void interp_##identifier(); \
    word_t word_##identifier = {prev_identifier, name, interp_##identifier}; \
    void interp_##identifier()
