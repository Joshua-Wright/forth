// (c) Copyright 2016 Josh Wright
#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define STACK_SIZE 5120
#define RETURN_STACK_SIZE 256

// assumes stacks grow down
#define PUSH(__stack, __data) ( *(--__stack) = (__data))
#define POP(__stack) ( *(__stack++) )

typedef struct _word_t word_t;
typedef struct _custom_word_t custom_word_t;
typedef void(*interpreter_t)();
typedef void(*compile_time_hook_t)(custom_word_t *custom_word, size_t *word_index);
typedef int64_t stack_t;
typedef struct _word_t {
    struct _word_t *prev;
    char *name;
    interpreter_t interpreter;
    compile_time_hook_t post_compile_hook;
} word_t;
typedef struct _custom_word_t {
    word_t word;
    // pointer to array of words that make up the function
    size_t code_size;
    word_t **code;
} custom_word_t;

typedef enum {
    CONTROL_IF,
    CONTROL_ELSE,
    CONTROL_DO,
} control_stack_type_t;
typedef struct {
    // type of unresolved control flow
    control_stack_type_t control_type;
    // where to put the resolved offset
    ssize_t *resolve_offset_dest;
    // offset is relative to the address of the ofset itself
} control_stack_t;

/**
 * stack grows down, and this points to bottom of the stack.
 * e.g. stack[0] is top, stack[1] is second to top
 * push to stack by:
 * stack--;
 * *stack=thing;
 */
extern stack_t *stack;
extern word_t ***return_stack;
extern word_t **prog_counter;
// tip of word linked-list
extern word_t *top_word;
extern control_stack_t *control_stack;

word_t *lookup_word(char *name);

void default_interpreter();

void forth_main_loop();

extern word_t word_literal;

void init_stdlib();

#define WORD_INTERP(__identifier) void interp_##__identifier()
#define WORD_COMPILE(__identifier) void compile_##__identifier(custom_word_t *custom_word, size_t *word_index)
#define WORD_STRUCT(__name, __identifier, __prev_identifier) \
    word_t word_##__identifier = {&word_##__prev_identifier, __name, interp_##__identifier, compile_##__identifier};
#define WORD_INTERP_ONLY(__name, __identifier, __prev_identifier) \
    WORD_INTERP(__identifier); \
    word_t word_##__identifier = {&word_##__prev_identifier, __name, interp_##__identifier, NULL}; \
    WORD_INTERP(__identifier)
//#define WORD_INTERP_ONLY(__name, __identifier, __prev_word) \
//    WORD_INTERP(__identifier); \
//    word_t word_##__identifier = {__prev_word, __name, interp_##__identifier, NULL}; \
//    WORD_INTERP(__identifier)
#define WORD_COMPILE_ONLY(__name, __identifier, __prev_word) \
    WORD_COMPILE(__identifier); \
    word_t word_##__identifier = {&word_##__prev_word, __name, NULL, compile_##__identifier }; \
    WORD_COMPILE(__identifier)
