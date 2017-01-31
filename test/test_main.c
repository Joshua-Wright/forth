// (c) Copyright 2016 Josh Wright

#include "minctest.h"
#include "../src/util.h"

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
    stack -= 5;
    stack[0] = 4;
    stack[1] = 3;
    stack[2] = 7;
    stack[3] = 9;
    stack[4] = 2;

//    struct _custom_word_t
}

int main(int argc, char const *argv[]) {
    lrun("add", add);
    lresults();
    return 0;
}
