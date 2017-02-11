// (c) Copyright 2016 Josh Wright

#include "minctest.h"
#include "../src/util.h"
#include "../src/eval.h"
#include "../src/debug_helpers.h"

void add() {
    stack -= 2;
    stack[0] = 1;
    stack[1] = 2;
    stack[2] = 8123;

    word_t *add = lookup_word("+");
    word_t *code[] = {add, add, NULL};
    prog_counter = code;
    // run to the end of the current block
    forth_main_loop();
    prog_counter = 0;
    lequal(stack[0], 1 + 2 + 8123);
}

void add_custom_func() {
    eval_str(": add4 + + + ;");
    stack -= 4;
    stack[0] = 4;
    stack[1] = 3;
    stack[2] = 7;
    stack[3] = 9;
    lequal(eval_str("add4"), 4 + 3 + 7 + 9);
}

void test_if() {
    lequal(eval_str("0 5 >"), 1);
    lequal(eval_str("5 0 <"), 1);

    // test if-else-then
    eval_str(": gt5 5 <= if 1 else 99 then ; ");
    lequal(eval_str("7 gt5 "), 1);
    lequal(eval_str("2 gt5 "), 99);

    // test if-else<nested>-then
    eval_str(": sgn "
                     " dup 0> if 1 else "
                     " dup 0< if -1 else "
                     " 0 "
                     " then then ; "
    );
    lequal(eval_str("3 sgn"), 1);
    lequal(eval_str("-3 sgn"), -1);
    lequal(eval_str("0 sgn"), 0);

    // test if-else-then-<other stuff>
    eval_str(": t1 0 if 2 else 3 then drop 5 ; ");
    lequal(eval_str("t1"), 5);

    // test if and then after each other
    eval_str(": t2 dup 0> if 1 * else 2 * then dup 0> if 1000 + else 2000 + then ; ");
    lequal(eval_str("0 t2"), 2000+0);
    lequal(eval_str("2 t2"), 1000+2);
    lequal(eval_str("7 t2"), 1000+7);
    lequal(eval_str("-2 t2"), 2000-2*2);
    lequal(eval_str("-7 t2"), 2000-7*2);
}

void fib() {
    eval_str(": fib dup 1 = if 1 else dup 2 = if 1 else dec dup dec fib swap fib + then then ; ");
    /* pretty_print_custom_word_by_name("fib"); */
    lequal(eval_str("1 fib"), 1);
    lequal(eval_str("2 fib"), 1);
    lequal(eval_str("3 fib"), 2);
    lequal(eval_str("4 fib"), 3);
    lequal(eval_str("5 fib"), 5);
    lequal(eval_str("7 fib"), 13);
}

void function_tests() {

    // arithmetic
    lequal(eval_str("3 4 +"), 3 + 4);
    lequal(eval_str("321 4523 -"), 4523 - 321);
    lequal(eval_str("8 3 * 321 4523 - /"), (4523 - 321) / (8 * 3));

    // stack manipulation
    lequal(eval_str("3 dup *"), 3 * 3);
    lequal(eval_str("3 dup dup * +"), 3 * 3 + 3);
    lequal(eval_str("3 987 /"), 987 / 3);
    lequal(eval_str("3 987 swap /"), 3/ 987);
    lequal(eval_str("3 987 drop"), 3);
}

int main() {
    init_stdlib();
    lrun("add", add);
    lrun("custom_func", add_custom_func);
    lrun("functions", function_tests);
    lrun("if", test_if);
//     lrun("fib", fib);
    lresults();
    return 0;
}
