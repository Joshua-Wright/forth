// (c) Copyright 2016 Josh Wright
#pragma once

#include "util.h"

// find the name of a word without the danger of dereferencing a bad pointer
char* safe_word_name(word_t *word);

void pretty_print_custom_word(custom_word_t *custom_word);
void pretty_print_custom_word_by_name(char *name);
