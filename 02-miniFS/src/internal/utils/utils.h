#ifndef UTILS_H
#define UTILS_H

/*
	Module with auxiliary utils
*/


// function splits string into the array of
// strings and returns it. also places item
// count to "count" pointer.
char **split_line(const char *data, int *count);


// function frees array of lines
void free_lines(char **lines, int count);

#endif