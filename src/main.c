#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sortings.h"
#include "logging_std_wrappers.h"

#define ARR_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

static const size_t MAX_INPUT_STRING_SIZE_WITH_SPECIAL_CHARS = MAX_INPUT_STRING_SIZE + 3; // including \r, \n, and \0

static const int EXPECTED_ARG_COUNT = 1 + 5;
static const int ARR_LENGTH_ARG_INDEX = 1;
static const int INPUT_FILE_ARG_INDEX = 2;
static const int OUTPUT_FILE_ARG_INDEX = 3;
static const int SORT_ALGORITHM_NAME_ARG_INDEX = 4;
static const int COMPARATOR_NAME_ARG_INDEX = 5;

typedef struct {
    char *name;
    void *ptr;
} NAMED_PTR;

void *get_ptr_by_name(const char *name, NAMED_PTR *named_ptrs, size_t named_ptr_count) {
    for (size_t i = 0; i < named_ptr_count; i++)
        if (strcmp(name, named_ptrs[i].name) == 0)
            return named_ptrs[i].ptr;
    return NULL;
}

void *logging_get_ptr_by_name(const char *name, NAMED_PTR *named_ptrs, size_t named_ptr_count, char *variable_name) {
    void *result = get_ptr_by_name(name, named_ptrs, named_ptr_count);
    if (result == NULL) {
        LOG_ERROR("Invalid %s. Expected one of the following: ", variable_name);
        for (size_t i = 0; i < named_ptr_count - 1; i++)
            LOG_ERROR("\"%s\", ", named_ptrs[i].name);
        if (named_ptr_count > 0) LOG_ERROR("\"%s\"", named_ptrs[named_ptr_count - 1].name);
        LOG_ERROR(". Found: \"%s\"\n", name);
        return NULL;
    }
    return result;
}

void free_str_arr(char **arr, size_t arr_length) {
    for (size_t i = 0; i < arr_length; i++)
        free(arr[i]);
    free(arr);
}

NAMED_PTR str_sort_funcs[] = {
        {"bubble",    bubble},
        {"insertion", insertion},
        {"merge",     merge},
        {"quick",     quick},
        {"radix",     radix}
};

NAMED_PTR comparator_funcs[] = {
        {"asc", cmp_string_ascending},
        {"des", cmp_string_descending}
};

char **read_str_arr(char *filepath, size_t arr_length) {
    FILE *file = logging_fopen(filepath, "r");
    if (file == NULL) return NULL;
    char **arr = logging_malloc(arr_length * sizeof(char *), "string array");
    if (arr == NULL) {
        fclose(file);
        return NULL;
    }
    for (size_t i = 0; i < arr_length; i++) {
        arr[i] = logging_malloc(MAX_INPUT_STRING_SIZE_WITH_SPECIAL_CHARS, "input string");
        if (arr[i] == NULL) {
            free_str_arr(arr, i);
            fclose(file);
            return NULL;
        }
        if (logging_fgets(arr[i], MAX_INPUT_STRING_SIZE_WITH_SPECIAL_CHARS, file, "input string") == NULL) {
            free_str_arr(arr, i + 1);
            fclose(file);
            return NULL;
        }
    }
    fclose(file);
    return arr;
}

int write_str_arr(char *filepath, char **arr, size_t arr_length) {
    FILE *file = logging_fopen(filepath, "w");
    if (file == NULL) return 1;
    for (size_t i = 0; i < arr_length; i++) {
        if (logging_fputs(arr[i], file, "input string") == EOF) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int str_to_arr_length(const char *str, array_size_t *result) {
    char *endptr;
    uintmax_t result_umax = strtoumax(str, &endptr, 0);
    if (endptr < str + strlen(str)) {
        LOG_ERROR("Invalid array length. \"%s\" is not a valid number\n", str);
        return 1;
    }
    if (result_umax == UINTMAX_MAX || result_umax > SIZE_MAX) {
        LOG_ERROR("Invalid array length. %s is too large\n", str);
        return 1;
    }
    *result = result_umax;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != EXPECTED_ARG_COUNT) {
        LOG_ERROR("Invalid number of arguments. Expected %d arg(s), but found %d arg(s)\n", EXPECTED_ARG_COUNT - 1, argc - 1);
        return 1;
    }
    array_size_t arr_length;
    if (str_to_arr_length(argv[ARR_LENGTH_ARG_INDEX], &arr_length) != 0) return 1;
    str_sort_func_t sort_func = logging_get_ptr_by_name(argv[SORT_ALGORITHM_NAME_ARG_INDEX], str_sort_funcs,
                                                        ARR_LENGTH(str_sort_funcs), "sorting algorithm");
    if (sort_func == NULL) return 1;
    comparator_func_t comparator = logging_get_ptr_by_name(argv[COMPARATOR_NAME_ARG_INDEX], comparator_funcs,
                                                           ARR_LENGTH(comparator_funcs), "comparator");
    if (comparator == NULL) return 1;
    char **arr = read_str_arr(argv[INPUT_FILE_ARG_INDEX], arr_length);
    if (arr == NULL) return 1;
    sort_func(arr, arr_length, comparator);
    if (get_sorting_exit_code() != 0) {
        free_str_arr(arr, arr_length);
        return 1;
    }
    int response = write_str_arr(argv[OUTPUT_FILE_ARG_INDEX], arr, arr_length);
    free_str_arr(arr, arr_length);
    return response;
}
