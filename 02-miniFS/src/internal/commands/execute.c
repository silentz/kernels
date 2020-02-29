#include <internal/commands/execute.h>
#include <internal/debug/debug.h>
#include <internal/fs/fs.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>


// defines

typedef void(*func_ptr)(Filesystem*, const char **, int);

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


void minifs_ls(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "ls command");
    DirectoryMap *content = minifs_read_dir(fs, fs->current_dir);
    for (uint32_t index = 0; index < content->size; ++index) {
        printf("%s ", content->names[index]);
    }
    printf("\n");
    minifs_clear_dirmap(content);
}


void minifs_cd(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "cd command");
}


void minifs_mkdir(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "mkdir command");
}


void minifs_rmdir(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "rmdir command");
}


void minifs_touch(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "touch command");
}


void minifs_rm(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "rm command");
}


void minifs_write(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "write command");
}


void minifs_read(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "read command");
}


void minifs_help(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "help command");
    printf("Command list:\n");
    int command_count = sizeof(commands) / sizeof(struct Command);
    for (int index = 0; index < command_count; ++index) {
        printf("%s\t - %s\n", commands[index].name, commands[index].description);
    }
}


void minifs_exit(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "exit command");
    close(fs->fd);
    exit(0);
}


void minifs_execute(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "execute function");
    if (count <= 0) {
        return;
    }
    int command_count = sizeof(commands) / sizeof(struct Command);
    for (int index = 0; index < command_count; ++index) {
        if (strcmp(data[0], commands[index].name) == 0) {
            commands[index].func(fs, data, count);
            return;
        }
    }
}