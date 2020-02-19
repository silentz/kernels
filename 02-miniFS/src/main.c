#include <stdio.h>


int main(int argc, char **argv) {
    // check if path to fs device is given
    if (argc != 2) {
        printf("[Error] format: %s <path/to/file>\n", argv[0]);
    }
    return 0;
}
