// (c) Copyright 2016 Josh Wright

#include <string.h>
#include <stdio.h>
#include <assert.h>
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
                    ": dec 1 - ; "
                    ": 0= 0 = ; "
                    ": 0> 0 > ; "
                    ": 0< 0 < ; "
                    ": 1- 1 - ; "
                    "";
    eval_str(lib);
}

////////////////////////////////////////////////////////////

void word_nop_func() {};


WORD_INTERP(not) {
    stack[0] = !stack[0];
}

word_t word_not = {
        .prev = NULL,
        .interpreter = interp_not,
        .name = "not",
        .post_compile_hook = NULL,
};

WORD_INTERP_ONLY("=", equal, not) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 == n2);
}

WORD_INTERP_ONLY(">=", greater_than_equal, equal) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 >= n2);
}

WORD_INTERP_ONLY("<=", less_than_eq, greater_than_equal) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 <= n2);
}

WORD_INTERP_ONLY("<", less_than, less_than_eq) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 < n2);
}

WORD_INTERP_ONLY(">", greater_than, less_than) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 > n2);
}


WORD_INTERP_ONLY("+", add, greater_than) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a + b);
}

WORD_INTERP_ONLY("-", sub, add) {
    stack_t n2 = POP(stack);
    stack_t n1 = POP(stack);
    PUSH(stack, n1 - n2);
}

WORD_INTERP_ONLY("*", mul, sub) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    PUSH(stack, a * b);
}

WORD_INTERP_ONLY("/", div, mul) {
    stack_t b = POP(stack);
    stack_t a = POP(stack);
    PUSH(stack, a / b);
}

WORD_INTERP_ONLY("drop", drop, div) {
    POP(stack);
}

WORD_INTERP_ONLY("dup", dup, drop) {
    stack--;
    stack[0] = stack[1];
}

WORD_INTERP_ONLY("2dup", 2dup, dup) {
    stack -= 2;
    stack[0] = stack[2];
    stack[1] = stack[3];
}

WORD_INTERP_ONLY("swap", swap, 2dup) {
    stack_t n1 = POP(stack);
    stack_t n2 = POP(stack);
    PUSH(stack, n1);
    PUSH(stack, n2);
}

WORD_INTERP_ONLY(".", print_signed_int, swap) {
    printf("%li\n", (long) stack[0]);
    stack++;
}

WORD_INTERP_ONLY("lit", literal, print_signed_int) {
    stack--;
    prog_counter++;
    stack[0] = (stack_t) prog_counter[0];
}

WORD_INTERP_ONLY("!", bang, literal) {
    int64_t *addr = (int64_t *) stack[0];
    int64_t x = stack[1];
    *addr = x;
    stack += 2;
}

WORD_INTERP_ONLY("over", over, bang) {
    stack--;
    stack[0] = stack[2];
}

WORD_INTERP_ONLY("rot", rot, over) {
    stack_t a = POP(stack);
    stack_t b = POP(stack);
    stack_t c = POP(stack);
    PUSH(stack, a);
    PUSH(stack, b);
    PUSH(stack, c);
}

WORD_INTERP_ONLY("jump", jump, rot) {
    // jumps to the address specified in extra code parameter
    prog_counter++;
    prog_counter += (int64_t) *prog_counter;
}

WORD_COMPILE_ONLY("recurse", recurse, rot) {
    custom_word->code[*word_index] = &custom_word->word;
//    (*word_index)++;
//    custom_word->code[*word_index] = (word_t *) -(*word_index);
}

WORD_INTERP_ONLY(";", ret, recurse) {
    prog_counter = POP(return_stack);
}

WORD_COMPILE_ONLY("exit", exit, ret) {
    custom_word->code[*word_index] = &word_ret;
}


WORD_COMPILE(branch_if_zero) {
    // increment so we're pointing at our branch offset location
    (*word_index)++;

    // push to control stack
    control_stack--;
    control_stack->control_type = CONTROL_IF;
    control_stack->resolve_offset_dest = (ssize_t *) &custom_word->code[*word_index];
}

WORD_INTERP(branch_if_zero) {
    stack_t cond = POP(stack);
    // increment PC so that we're now pointing at the offset
    prog_counter++;
    if (cond == 0) {
        prog_counter += (int64_t) prog_counter[0];
    }
}

WORD_STRUCT("if", branch_if_zero, exit);


WORD_COMPILE(else) {
    assert(control_stack->control_type == CONTROL_IF);

    // increment so we're pointing at our branch offset
    (*word_index)++;
    // resolve if condition
    ssize_t *addr_of_if = control_stack->resolve_offset_dest;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    *(control_stack->resolve_offset_dest) = addr_of_this - addr_of_if;
    // just replace control stack with our branch offset
    control_stack->control_type = CONTROL_ELSE;
    control_stack->resolve_offset_dest = (ssize_t *) &custom_word->code[*word_index];
};

WORD_INTERP(else) {
    // jumps to the address specified in extra code parameter
    prog_counter++;
    prog_counter += (int64_t) *prog_counter;
}

WORD_STRUCT("else", else, branch_if_zero);


WORD_COMPILE_ONLY("then", then, else) {
    assert(control_stack->control_type == CONTROL_IF || control_stack->control_type == CONTROL_ELSE);
    // don't bother actually putting a then in the code because it does nothing anyway
    (*word_index)--;
    // pop reference from control stack
    ssize_t *addr_of_if = control_stack->resolve_offset_dest;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    // -1 because we don't have any branch offset of our own to skip past
    *(control_stack->resolve_offset_dest) = addr_of_this - addr_of_if;
    control_stack++;
}


WORD_COMPILE(do) {
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];

    control_stack--;
    control_stack->resolve_offset_dest = addr_of_this;
    control_stack->control_type = CONTROL_DO;
}

WORD_INTERP(do) {
    // keep looping info on the return stack
    return_stack -= 2;
    // index
    return_stack[0] = (word_t **) POP(stack);
    // limit
    return_stack[1] = (word_t **) POP(stack);
}

WORD_STRUCT("do", do, then);


WORD_COMPILE(loop) {
    // advance word counter to point at our offset
    (*word_index)++;

    assert(control_stack->control_type == CONTROL_DO);
    // keep the address of the do block to return to
    ssize_t *addr_of_do = control_stack->resolve_offset_dest;
    control_stack++;
    ssize_t *addr_of_this = (ssize_t *) &custom_word->code[*word_index];
    // set our offset
    custom_word->code[*word_index] = (word_t *) (addr_of_do - addr_of_this);
}

WORD_INTERP(loop) {
    // fetch index and limit
    uint64_t index = (uint64_t) return_stack[0];
    uint64_t limit = (uint64_t) return_stack[1];

    // we can't increment the index in-place because that would be using pointer math
    // so we increment it and then put it back
    index++;
    return_stack[0] = (word_t **) index;

    // increment so we're looking at our branch target address
    prog_counter++;
    if (index < limit) {
        // retrun to loop start
        prog_counter += (int64_t) prog_counter[0];
    } else {
        // pop loop info and keep going
        return_stack += 2;
    }
}

WORD_STRUCT("loop", loop, do)

WORD_INTERP_ONLY("i", get_loop_counter, loop) {
    PUSH(stack, return_stack[0]);
}

WORD_INTERP_ONLY("j", get_loop_counter2, get_loop_counter) {
    PUSH(stack, return_stack[2]);
}


word_t *top_word = &word_get_loop_counter2;
