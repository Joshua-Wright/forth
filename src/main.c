#include <stdio.h>
#include "eval.h"

int main() {
    init_stdlib();
    // TODO fancy repl session
    eval_file(stdin);
    return 0;
}
