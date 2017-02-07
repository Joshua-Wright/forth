// (c) Copyright 2016 Josh Wright

#include <string.h>
#include <stdio.h>
#include "util.h"
#include "eval.h"
#include "debug_helpers.h"

static stack_t __stack[STACK_SIZE] = {0};
stack_t *stack = ((stack_t *) __stack) + STACK_SIZE - 1;

static word_t **__return_stack[RETURN_STACK_SIZE] = {0};
word_t ***return_stack = ((word_t ***) __return_stack) + RETURN_STACK_SIZE - 1;

static control_stack_t __control_stack[STACK_SIZE] = {0};
control_stack_t *control_stack = ((control_stack_t *) __control_stack) + STACK_SIZE - 1;


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
                    ": 0= 0 = ; "
                    ": 0> 0 < ; "
                    ": 0< 0 > ; "
                    "";
    eval_str(lib);
}

////////////////////////////////////////////////////////////

void word_nop_func() {};

DECLARE_WORD("not", not, NULL) { stack[0] = !stack[0]; }

DECLARE_WORD("=", equal, &word_not) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 == n2);
}

DECLARE_WORD(">=", greater_than_equal, &word_equal) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 >= n2);
}

DECLARE_WORD("<=", less_than_eq, &word_greater_than_equal) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 <= n2);
}

DECLARE_WORD("<", less_than, &word_less_than_eq) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 < n2);
}

DECLARE_WORD(">", greater_than, &word_less_than) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 > n2);
}


DECLARE_WORD("+", add, &word_greater_than) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a + b);
}

DECLARE_WORD("-", sub, &word_add) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a - b);
}

DECLARE_WORD("*", mul, &word_sub) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a * b);
}

DECLARE_WORD("/", div, &word_mul) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a / b);
}

DECLARE_WORD("drop", drop, &word_div) { POP(stack); }

DECLARE_WORD("dup", dup, &word_drop) {
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
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    stack_t c = POP(stack);
    PUSH(stack, a);
    PUSH(stack, b);
    PUSH(stack, c);
}

DECLARE_WORD("jump", jump, &word_rot) {
    // jumps to the address specified in extra code parameter
    prog_counter++;
    prog_counter += (int64_t) *prog_counter;
}


void interp_branch_if_zero() {
    stack_t cond = POP(stack);
    // increment PC so that we're now pointing at the offset
    prog_counter++;
    if (cond == 0) {
        prog_counter += (int64_t) prog_counter[0];
    }
}

void compile_branch_if_zero(custom_word_t *custom_word, size_t *word_index) {
    // push to control stack
    control_stack--;
    control_stack->control_type = CONTROL_IF;
    control_stack->resolve_offset_dest = (ssize_t *) &custom_word->code[*word_index];

    // increment so the next instruction goes after our branch offset
    (*word_index)++;
}

word_t word_branch_if_zero = {
        .prev = &word_jump,
        .name = "if",
        .interpreter = interp_branch_if_zero,
        .post_compile_hook = compile_branch_if_zero,
};


void compile_else(custom_word_t *custom_word, size_t *word_index) {
    // resolve if condition
    ssize_t *addr_of_if = control_stack->resolve_offset_dest;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    *(control_stack->resolve_offset_dest) = addr_of_this - addr_of_if;
    // just replace control stack with our branch offset
    control_stack->control_type = CONTROL_ELSE;
    control_stack->resolve_offset_dest = (ssize_t *) &custom_word->code[*word_index];
    // increment so the next instruction goes after our branch offset
    (*word_index)++;
};

word_t word_else = {
        .prev = &word_branch_if_zero,
        .name = "else",
        // if we get here, it's because we continued from the top of the if
        // so skip past our expression body
        .interpreter = interp_jump,
        .post_compile_hook = compile_else,
};

void compile_then(custom_word_t *custom_word, size_t *word_index) {
    // pop reference from control stack and insert offset to us
//    *control_stack->resolve_offset_dest = ((ssize_t) &custom_word->code[*word_index]) - ((ssize_t) control_stack->resolve_offset_dest);
    ssize_t *addr_of_if = control_stack->resolve_offset_dest;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    // -1 because we don't have any branch offset of our own to skip past
    *(control_stack->resolve_offset_dest) = addr_of_this - addr_of_if - 1;
    control_stack++;
}

word_t word_then = {
        .prev = &word_else,
        .name = "then",
        .interpreter = word_nop_func,
        .post_compile_hook = compile_then,
};


word_t *top_word = &word_then;
