// (c) Copyright 2016 Josh Wright

#include "minctest.h"
#include "../src/util.h"
#include "../src/eval.h"

void add() {
    stack -= 2;
    stack[0] = 1;
    stack[1] = 2;
    stack[2] = 8123;

    word_t *add = lookup_word("+");
    word_t *code[] = {add, add, NULL};
    prog_counter = code;
    // run to the end of the current block
    do {
        call();
    } while (prog_counter[0] != NULL);

    lequal(stack[0], 1 + 2 + 8123);
}

void add_custom_func() {
    stack -= 4;
    stack[0] = 4;
    stack[1] = 3;
    stack[2] = 7;
    stack[3] = 9;
    eval_str(": add4 + + + ;");
    eval_str("add4");
    lequal(stack[0], 4 + 3 + 7 + 9);
}

void function_tests() {
#define DO_TEST(in, stack_top) \
    eval_str(in); \
    lequal(stack[0], stack_top);

    // arithmetic
    DO_TEST("3 4 +", 3 + 4);
    DO_TEST("321 4523 -", 4523 - 321);
    DO_TEST("321 4523 - 8 3 * /", (4523 - 321) / (8 * 3));

    // stack manipulation
    DO_TEST("3 dup *", 3 * 3);
    DO_TEST("3 dup dup * +", 3 * 3 + 3);
    DO_TEST("3 987 swap /", 987 / 3);
}

int main() {
    init_stdlib();
    lrun("add", add);
    lrun("custom_func", add_custom_func);
    lrun("functions", function_tests);
    lresults();
    return 0;
}
