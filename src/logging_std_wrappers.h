#pragma once

#include <stdio.h>
#include "stddef.h"

#define LOG_ERROR(...) (fprintf(stderr, __VA_ARGS__))

void *logging_malloc(size_t block_size, const char *variable_name);
FILE *logging_fopen(const char *filename, const char *mode);
char *logging_fgets(char *str, int num, FILE *stream, const char *variable_name);
int logging_fputs(char *str, FILE *stream, const char *variable_name);
