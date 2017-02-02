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
    prog_counter = 0;

    lequal(stack[0], 1 + 2 + 8123);
}

void add_custom_func() {
    stack -= 4;
    stack[0] = 4;
    stack[1] = 3;
    stack[2] = 7;
    stack[3] = 9;
    eval_str(": add4 + + + ;");
    lequal(eval_str("add4"), 4 + 3 + 7 + 9);
}

void test_if() {
    lequal(eval_str("5 0 >"), 1);
    lequal(eval_str("0 5 <"), 1);

    eval_str(": gt5 5 > if 1 else 0 then ; ");
    lequal(eval_str("7 gt5 "), 1);
    lequal(eval_str("2 gt5 "), 0);

    eval_str(": sgn "
                     " dup 0> if 1 else "
                     " dup 0< if -1 else "
                     " 0 "
                     " then then ; "
    );
    lequal(eval_str("3 sgn"), 1);
    lequal(eval_str("-3 sgn"), -1);
    lequal(eval_str("0 sgn"), 0);
}

void function_tests() {

    // arithmetic
    lequal(eval_str("3 4 +"), 3 + 4);
    lequal(eval_str("321 4523 -"), 4523 - 321);
    lequal(eval_str("321 4523 - 8 3 * /"), (4523 - 321) / (8 * 3));

    // stack manipulation
    lequal(eval_str("3 dup *"), 3 * 3);
    lequal(eval_str("3 dup dup * +"), 3 * 3 + 3);
    lequal(eval_str("3 987 swap /"), 987 / 3);
    lequal(eval_str("3 987 drop"), 3);
}

int main() {
    init_stdlib();
    lrun("add", add);
    lrun("custom_func", add_custom_func);
    lrun("functions", function_tests);
    lrun("if", test_if);
    lresults();
    return 0;
}
