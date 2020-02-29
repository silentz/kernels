#include <internal/commands/execute.h>
#include <internal/debug/debug.h>
#include <stdio.h>
#include <string.h>


// defines

typedef void(*func_ptr)(const char **, int);

static struct Command {
    const char *name;
    const char *description;
    func_ptr func;
} commands[] = {
    {
        .name = "cd",
        .description = "change directory",
        .func = minifs_cd
    },
    {
        .name = "ls",
        .description = "list directory",
        .func = minifs_ls
    },
    {
        .name = "mkdir",
        .description = "create directory",
        .func = minifs_mkdir
    },
    {
        .name = "rmdir",
        .description = "remove directory",
        .func = minifs_rmdir
    },
    {
        .name = "touch",
        .description = "create file",
        .func = minifs_touch
    },
    {
        .name = "rm",
        .description = "remove file",
        .func = minifs_rm
    },
    {
        .name = "write",
        .description = "write to file",
        .func = minifs_write
    },
    {
        .name = "read",
        .description = "read from file",
        .func = minifs_read
    },
    {
        .name = "help",
        .description = "print help menu",
        .func = minifs_help
    },
    {
        .name = "exit",
        .description = "quit minifs",
        .func = minifs_exit
    }
};


void minifs_ls(const char **data, int count) {
    debug(MINIFS_INFO "ls command");
}


void minifs_cd(const char **data, int count) {
    debug(MINIFS_INFO "cd command");
}


void minifs_mkdir(const char **data, int count) {
    debug(MINIFS_INFO "mkdir command");
}


void minifs_rmdir(const char **data, int count) {
    debug(MINIFS_INFO "rmdir command");
}


void minifs_touch(const char **data, int count) {
    debug(MINIFS_INFO "touch command");
}


void minifs_rm(const char **data, int count) {
    debug(MINIFS_INFO "rm command");
}


void minifs_write(const char **data, int count) {
    debug(MINIFS_INFO "write command");
}


void minifs_read(const char **data, int count) {
    debug(MINIFS_INFO "read command");
}


void minifs_help(const char **data, int count) {
    debug(MINIFS_INFO "help command");
    printf("Command list:\n");
    int command_count = sizeof(commands) / sizeof(struct Command);
    for (int index = 0; index < command_count; ++index) {
        printf("%s\t - %s\n", commands[index].name, commands[index].description);
    }
}


void minifs_exit(const char **data, int count) {
    debug(MINIFS_INFO "exit command");
}


void minifs_execute(const char **data, int count) {
    debug(MINIFS_INFO "execute function");
    if (count <= 0) {
        return;
    }
    int command_count = sizeof(commands) / sizeof(struct Command);
    for (int index = 0; index < command_count; ++index) {
        if (strcmp(data[0], commands[index].name) == 0) {
            commands[index].func(data, count);
            return;
        }
    }
}