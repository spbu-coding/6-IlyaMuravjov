#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "sortings.h"
#include "logging_std_wrappers.h"

static int sorting_exit_code = 0;

int get_sorting_exit_code() {
    return sorting_exit_code;
}

int cmp_string_ascending(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

int cmp_string_descending(const char *s1, const char *s2) {
    return -cmp_string_ascending(s1, s2);
}

void swap_strings(char **s1, char **s2) {
    char *temp = *s1;
    *s1 = *s2;
    *s2 = temp;
}

void swap_strings_in_arr(char **arr, array_size_t i, array_size_t j) {
    swap_strings(arr + i, arr + j);
}

void bubble(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator) {
    sorting_exit_code = 0;
    for (array_size_t i = 0; i < arr_length; i++)
        for (array_size_t j = 0; j < arr_length - i - 1; j++)
            if (comparator(arr[j], arr[j + 1]) > 0)
                swap_strings_in_arr(arr, j, j + 1);
}

void insertion(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator) {
    sorting_exit_code = 0;
    for (array_size_t i = 1; i < arr_length; i++) {
        char *cur = arr[i];
        array_size_t j;
        for (j = i; j > 0 && comparator(cur, arr[j - 1]) < 0; j--)
            arr[j] = arr[j - 1];
        arr[j] = cur;
    }
}

void merge_arrays(char **arr1_begin, char **arr1_end,
                  char **arr2_begin, char **arr2_end,
                  comparator_func_t comparator, char **result) {
    while (arr1_begin < arr1_end && arr2_begin < arr2_end)
        *result++ = comparator(*arr1_begin, *arr2_begin) < 0 ? *arr1_begin++ : *arr2_begin++;
    while (arr1_begin < arr1_end) *result++ = *arr1_begin++;
    while (arr2_begin < arr2_end) *result++ = *arr2_begin++;
}

void recursive_merge_sort(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator, char **work_space) {
    if (arr_length <= 1) return;
    array_size_t mid = arr_length / 2u;
    strings_array_t arr_second_half = arr + mid;
    recursive_merge_sort(arr, mid, comparator, work_space);
    recursive_merge_sort(arr_second_half, arr_length - mid, comparator, work_space);
    merge_arrays(arr, arr_second_half, arr_second_half, arr + arr_length, comparator, work_space);
    memcpy(arr, work_space, arr_length * sizeof(char *));
}

void merge(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator) {
    sorting_exit_code = 0;
    char **work_space = logging_malloc(arr_length * sizeof(char *), "merge sorting work space");
    if (work_space == NULL) {
        sorting_exit_code = 1;
        return;
    }
    recursive_merge_sort(arr, arr_length, comparator, work_space);
    free(work_space);
}

void shuffle_string_arr(strings_array_t arr, array_size_t arr_length) {
    for (size_t i = 0; i < arr_length; i++)
        swap_strings_in_arr(arr, i, rand() / ((RAND_MAX / arr_length) + 1));
}

void recursive_quick_sort(char **first, char **last, comparator_func_t comparator) {
    if (first >= last) return;
    char **pivot = first;
    char **left = first;
    char **right = last;
    while (left < right) {
        while (comparator(*left, *pivot) <= 0 && left < last) left++;
        while (comparator(*right, *pivot) > 0) right--;
        if (left < right) swap_strings(left, right);
    }
    swap_strings(right, pivot);
    recursive_quick_sort(first, right - 1, comparator);
    recursive_quick_sort(right + 1, last, comparator);
}

void quick(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator) {
    sorting_exit_code = 0;
    shuffle_string_arr(arr, arr_length);
    recursive_quick_sort(arr, arr + arr_length - 1, comparator);
}

void free_string_buckets(char **buckets[], size_t bucket_count) {
    for (size_t i = 0; i < bucket_count; i++)
        free(buckets[i]);
}

void radix(strings_array_t arr, array_size_t arr_length, comparator_func_t comparator) {
    sorting_exit_code = 0;
    static const size_t ALPHABET_LENGTH = UCHAR_MAX + 1;
    char alphabet[ALPHABET_LENGTH][2];
    char **buckets[ALPHABET_LENGTH];
    array_size_t bucket_sizes[ALPHABET_LENGTH];
    for (size_t i = 0; i < ALPHABET_LENGTH; i++) {
        buckets[i] = logging_malloc(arr_length * sizeof(char **), "radix sorting bucket");
        if (buckets[i] == NULL) {
            free_string_buckets(buckets, i);
            sorting_exit_code = 1;
            return;
        }
        alphabet[i][0] = i;
        alphabet[i][1] = '\0';
    }
    qsort(alphabet, ALPHABET_LENGTH, sizeof(alphabet[0]), (__compar_fn_t) comparator);
    for (size_t char_index = MAX_INPUT_STRING_SIZE - 1; char_index < SIZE_MAX; char_index--) {
        memset(bucket_sizes, 0, ALPHABET_LENGTH * sizeof(array_size_t));
        for (array_size_t str_index = 0; str_index < arr_length; str_index++) {
            unsigned char bucket_index = arr[str_index][char_index];
            buckets[bucket_index][bucket_sizes[bucket_index]++] = arr[str_index];
        }
        array_size_t str_index = 0;
        for (size_t i = 0; i < ALPHABET_LENGTH; i++) {
            unsigned char bucket_index = alphabet[i][0];
            for (array_size_t j = 0; j < bucket_sizes[bucket_index]; j++)
                arr[str_index++] = buckets[bucket_index][j];
        }
    }
    free_string_buckets(buckets, ALPHABET_LENGTH);
}
