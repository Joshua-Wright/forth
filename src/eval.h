// (c) Copyright 2016 Josh Wright
#pragma once

#include <stdio.h>
#include "util.h"

void eval_file(FILE *fp);

/**
 * @param str code to evaluate
 * @return top of stack, for convenience (destructively POP()ed)
 */
stack_t eval_str(const char *str);