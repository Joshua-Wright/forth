// (c) Copyright 2016 Josh Wright

#include <string.h>
#include <stdio.h>
#include "util.h"
#include "eval.h"
#include "debug_helpers.h"

#define DECLARE_STACK(__size, __type) \
    ( ((__type*)                      \
       (&(__type[__size]){0}))        \
      + __size - 1)

stack_t *stack = DECLARE_STACK(STACK_SIZE, stack_t);
word_t ***return_stack = DECLARE_STACK(RETURN_STACK_SIZE, word_t**);
control_stack_t *control_stack = DECLARE_STACK(STACK_SIZE, control_stack_t);


word_t **prog_counter = 0;

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
    PUSH(return_stack, prog_counter);

    // find code and set prog counter
    custom_word_t *custom_word = (custom_word_t *) *prog_counter;
    prog_counter = &custom_word->code[-1];
}


void forth_main_loop() {
    // run code
    do {
        word_t *word = *prog_counter;
        word->interpreter();
        prog_counter++;
    } while (prog_counter[0] != NULL);
    // ret function handles returning from functions
}


void init_stdlib() {
    const char *lib =
            ": inc 1 + ; "
                    ": dec -1 + ; "
                    ": 0= 0 = ; "
                    ": 0> 0 < ; "
                    ": 0< 0 > ; "
                    "";
    eval_str(lib);
}

////////////////////////////////////////////////////////////

void word_nop_func() {};

WORD_INTERP_ONLY("not", not, NULL) { stack[0] = !stack[0]; }

WORD_INTERP_ONLY("=", equal, &word_not) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 == n2);
}

WORD_INTERP_ONLY(">=", greater_than_equal, &word_equal) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 >= n2);
}

WORD_INTERP_ONLY("<=", less_than_eq, &word_greater_than_equal) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 <= n2);
}

WORD_INTERP_ONLY("<", less_than, &word_less_than_eq) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 < n2);
}

WORD_INTERP_ONLY(">", greater_than, &word_less_than) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 > n2);
}


WORD_INTERP_ONLY("+", add, &word_greater_than) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a + b);
}

WORD_INTERP_ONLY("-", sub, &word_add) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1 - n2);
}

WORD_INTERP_ONLY("*", mul, &word_sub) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a * b);
}

WORD_INTERP_ONLY("/", div, &word_mul) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a / b);
}

WORD_INTERP_ONLY("drop", drop, &word_div) { POP(stack); }

WORD_INTERP_ONLY("dup", dup, &word_drop) {
    stack--;
    stack[0] = stack[1];
}

WORD_INTERP_ONLY("dup2", dup2, &word_dup) {
    stack -= 2;
    stack[0] = stack[2];
    stack[1] = stack[3];
}

WORD_INTERP_ONLY("swap", swap, &word_dup2) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1);
    PUSH(stack, n2);
}

WORD_INTERP_ONLY(".", print_signed_int, &word_swap) {
    printf("%li\n", (long) stack[0]);
    stack++;
}

WORD_INTERP_ONLY("lit", literal, &word_print_signed_int) {
    stack--;
    prog_counter++;
    stack[0] = (stack_t) prog_counter[0];
}

WORD_INTERP_ONLY("!", bang, &word_literal) {
    int64_t *addr = (int64_t *) stack[0];
    int64_t x = stack[1];
    *addr = x;
    stack += 2;
}

WORD_INTERP_ONLY("over", over, &word_bang) {
    stack--;
    stack[0] = stack[2];
}

WORD_INTERP_ONLY("rot", rot, &word_over) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    stack_t c = POP(stack);
    PUSH(stack, a);
    PUSH(stack, b);
    PUSH(stack, c);
}

WORD_INTERP_ONLY("jump", jump, &word_rot) {
    // jumps to the address specified in extra code parameter
    prog_counter++;
    prog_counter += (int64_t) *prog_counter;
}

WORD_INTERP_ONLY(";", ret, &word_jump) {
    prog_counter = POP(return_stack);
}


WORD_COMPILE(branch_if_zero) {
    // push to control stack
    control_stack--;
    control_stack->control_type = CONTROL_IF;
    control_stack->resolve_offset_dest = (ssize_t *) &custom_word->code[*word_index];

    // increment so the next instruction goes after our branch offset
    (*word_index)++;
}

WORD_INTERP(branch_if_zero) {
    stack_t cond = POP(stack);
    // increment PC so that we're now pointing at the offset
    prog_counter++;
    if (cond == 0) {
        prog_counter += (int64_t) prog_counter[0];
    }
}

WORD_STRUCT("if", branch_if_zero, ret);


WORD_COMPILE(else) {
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

WORD_INTERP(else) {
    // jumps to the address specified in extra code parameter
    prog_counter++;
    prog_counter += (int64_t) *prog_counter;
}

WORD_STRUCT("else", else, branch_if_zero);


WORD_COMPILE(then) {
    // pop reference from control stack and insert offset to us
//    *control_stack->resolve_offset_dest = ((ssize_t) &custom_word->code[*word_index]) - ((ssize_t) control_stack->resolve_offset_dest);
    ssize_t *addr_of_if = control_stack->resolve_offset_dest;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    // -1 because we don't have any branch offset of our own to skip past
    *(control_stack->resolve_offset_dest) = addr_of_this - addr_of_if - 1;
    control_stack++;
}

WORD_INTERP(then) {}

WORD_STRUCT("then", then, else);


word_t *top_word = &word_then;
