#ifndef PARSER_H
#define PARSER_H

#include <internal/fs/fs.h>

// commands
void minifs_ls(Filesystem*, const char **, int);
void minifs_cd(Filesystem*, const char **, int);
void minifs_mkdir(Filesystem*, const char **, int);
void minifs_rmdir(Filesystem*, const char **, int);
void minifs_touch(Filesystem*, const char **, int);
void minifs_rm(Filesystem*, const char **, int);
void minifs_write(Filesystem*, const char **, int);
void minifs_read(Filesystem*, const char **, int);
void minifs_help(Filesystem*, const char **, int);
void minifs_exit(Filesystem*, const char **, int);
void minifs_debug(Filesystem*, const char **, int);

// main function that executes other commands or throws error
void minifs_execute(Filesystem*, const char **, int);

#endif