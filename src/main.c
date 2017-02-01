#include <stdio.h>
#include "eval.h"

int main() {
    init_stdlib();
    eval_file(stdin);
    return 0;
}
