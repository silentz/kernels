#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <internal/utils/utils.h>
#include <internal/commands/execute.h>


int main(int argc, char **argv) {
	char *input; 	// input line
	char **tokens;	// input split lines
	int count;		// count of input split lines

    if (argc != 2) {   // check if path to fs device is given
        printf("[Error] format: %s <path/to/file>\n", argv[0]);
        return -1;
    }

    while (true) {
        input = readline("$ ");
        tokens = split_line(input, &count);
		minifs_execute(tokens, count);
        free_lines(tokens, count);
        free(input);
    }

    return 0;
}
