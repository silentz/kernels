#include <internal/utils/utils.h>
#include <stdio.h>
#include <stdbool.h>


bool test_split_lines();


int main() {
    bool global = true;
    global &= test_split_lines();

    if (global) {
        printf("[GLOBAL OK]\n");
    }

    return 0;
}


// =========== [ TESTS ] ===========

bool test_split_lines() {
    char *data;
    char **lines;
    int count;
    bool status;

    status = true;

    data = "test1";
    lines = split_line(data, &count);
    if (count != 1) {
        status = false;
        printf("[BAD] 1 test_split_lines\n");
    }
    if (strcmp(lines[0], "test1") != 0) {
        status = false;
        printf("[BAD] 1 test_split_lines\n");
    }
    free_lines(lines, count);

    data = "test2.0 test2.1";
    lines = split_line(data, &count);
    if (count != 2) {
        status = false;
        printf("[BAD] 2 test_split_lines\n");
    }
    if (strcmp(lines[0], "test2.0") != 0) {
        status = false;
        printf("[BAD] 2 test_split_lines\n");
    }
    if (strcmp(lines[1], "test2.1") != 0) {
        status = false;
        printf("[BAD] 2 test_split_lines\n");
    }
    free_lines(lines, count);

    data = "  testa   testb";
    lines = split_line(data, &count);
    if (count != 2) {
        status = false;
        printf("[BAD] 3 test_split_lines\n");
    }
    if (strcmp(lines[0], "testa") != 0) {
        status = false;
        printf("[BAD] 3 test_split_lines\n");
    }
    if (strcmp(lines[1], "testb") != 0) {
        status = false;
        printf("[BAD] 3 test_split_lines\n");
    }
    free_lines(lines, count);

    data = "  testa     testb      ";
    lines = split_line(data, &count);
    if (count != 2) {
        status = false;
        printf("[BAD] 4 test_split_lines\n");
    }
    if (strcmp(lines[0], "testa") != 0) {
        status = false;
        printf("[BAD] 4 test_split_lines\n");
    }
    if (strcmp(lines[1], "testb") != 0) {
        status = false;
        printf("[BAD] 4 test_split_lines\n");
    }
    free_lines(lines, count);

    if (status) {
        printf("[OK] test_split_lines\n");
    } else {
        printf("[BAD] test_split_lines\n");
    }

    return status;
}