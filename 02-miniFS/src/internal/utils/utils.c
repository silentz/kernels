#include <internal/utils/utils.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


char **split_line(const char *data, int *count) {
	char **result;  	// result tokens of data
	int length; 		// length of string;
	const char *begin; 	// pointer to begin of token
	int index;			// loop variab;e
	
	length = strlen(data);
	*count = 0;

	if (length == 0) { 	// check if data is empty
		return NULL;
	}

	for (index = 1; index < length; ++index) {
		if (isspace(data[index]) && !isspace(data[index - 1])) {
			++(*count);
		}
	}
	if (!isspace(data[length - 1])) { 	// increase counter if data ends with non-space char
		++(*count);
	}

	result = (char**) malloc(sizeof(char*) * (*count));
	index = 0;

	for (index = 0; index < (*count); ++index) {
		while (isspace(*data)) { 	// skip leading spaces
			++data;
		}
		begin = data;
		while (!isspace(*data) && (*data != '\0')) {
			++data;
		}
		result[index] = (char*) malloc(sizeof(char) * (data - begin + 1));
		memcpy(result[index], begin, data - begin);
		result[index][data - begin] = '\0';
	}

	return result;
}


void free_lines(char **lines, int count) {
	for (int index = 0; index < count; ++index) {
		free(lines[index]);
	}
	free(lines);
}