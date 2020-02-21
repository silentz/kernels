#ifndef PARSER_H
#define PARSER_H

// commands
void minifs_ls(const char **, int);
void minifs_cd(const char **, int);
void minifs_mkdir(const char **, int);
void minifs_rmdir(const char **, int);
void minifs_touch(const char **, int);
void minifs_rm(const char **, int);
void minifs_write(const char **, int);
void minifs_read(const char **, int);
void minifs_help(const char **, int);
void minifs_exit(const char **, int);

// main function that executes other commands or throws error
void minifs_execute(const char **, int);

#endif