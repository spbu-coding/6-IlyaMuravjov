#include <malloc.h>
#include <stdio.h>
#include "logging_std_wrappers.h"

void log_file_reading_error(FILE *stream, const char *variable_name) {
    LOG_ERROR(feof(stream) ? "Unexpected end of file while reading %s\n" : "Unable to read %s\n", variable_name);
}

void log_file_writing_error(const char *variable_name) {
    LOG_ERROR("Unable to write %s\n", variable_name);
}

void *logging_malloc(size_t block_size, const char *variable_name) {
    void *allocated_block = malloc(block_size);
    if (allocated_block == NULL) LOG_ERROR("Unable to allocate memory for %s\n", variable_name);
    return allocated_block;
}

FILE *logging_fopen(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) LOG_ERROR("Unable to find file \"%s\"\n", filename);
    return file;
}

char *logging_fgets(char *str, int num, FILE *stream, const char *variable_name) {
    char *result = fgets(str, num, stream);
    if (result == NULL) log_file_reading_error(stream, variable_name);
    return result;
}

int logging_fputs(char *str, FILE *stream, const char *variable_name) {
    int result = fputs(str, stream);
    if (result == EOF) log_file_writing_error(variable_name);
    return result;
}
