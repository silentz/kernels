#include <internal/commands/execute.h>
#include <internal/debug/debug.h>
#include <internal/fs/fs.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>


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
    },
    {
        .name = "debug",
        .description = "debug fs",
        .func = minifs_debug
    }
};


void minifs_ls(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "ls command");
    DirectoryMap *content = minifs_read_dir(fs, fs->current_dir);
    printf("\e[34m.\e[0m \e[34m..\e[0m ");
    for (uint32_t index = 0; index < content->size; ++index) {
        switch (fs->sblock.inode_map[content->inodes[index]].type) {
            case MINIFS_INODE_DIRECTORY:
                printf("\e[34m%s\e[0m ", content->names[index]);
                break;
            case MINIFS_INODE_FILE:
                printf("%s ", content->names[index]);
                break;
            default:
                break;
        }
    }
    printf("\n");
    minifs_clear_dirmap(content);
}


void minifs_cd(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "cd command");
    if (count < 2) {
        fprintf(stderr, "format: %s <dirname>\n", data[0]);
        return;
    }

    if (strcmp(data[1], ".") == 0) {
        return;
    }

    if (strcmp(data[1], "..") == 0) {
        Inode c_inode = fs->sblock.inode_map[fs->current_dir];
        fs->current_dir = c_inode.parent;
        return;
    }

    DirectoryMap *content = minifs_read_dir(fs, fs->current_dir);
    for (uint32_t index = 0; index < content->size; ++index) {
        Inode inode = fs->sblock.inode_map[content->inodes[index]];
        if (inode.type == MINIFS_INODE_DIRECTORY) {
            if (strcmp(content->names[index], data[1]) == 0) {
                fs->current_dir = content->inodes[index];
                minifs_clear_dirmap(content);
                return;
            }
        }
    }

    fprintf(stderr, "directory not found\n");
    minifs_clear_dirmap(content);
}


void minifs_mkdir(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "mkdir command");
    if (count < 2) {
        fprintf(stderr, "format: %s <dirname>\n", data[0]);
        return;
    }
    if (strlen(data[1]) > MAX_FILENAME_SIZE) {
        fprintf(stderr, "filename is too long\n");
        return;
    }
    int32_t inode_index = minifs_find_free_inode(fs);
    int32_t block_index = minifs_find_free_block(fs);
    if (inode_index < 0) {
        fprintf(stderr, "Ran out of free inodes\n");
        return;
    }
    if (block_index < 0) {
        fprintf(stderr, "Ran out of free blocks\n");
        return;
    }

    fs->sblock.inode_map[inode_index].type = MINIFS_INODE_DIRECTORY;
    fs->sblock.inode_map[inode_index].root_block = block_index;
    fs->sblock.inode_map[inode_index].parent = fs->current_dir;
    fs->sblock.inode_map[inode_index].size = 0;
    
    fs->sblock.block_map[block_index].size = 0;
    fs->sblock.block_map[block_index].next_block = -1;
    fs->sblock.block_map[block_index].type = MINIFS_BLOCK_USED;

    fs->sblock.used_inode_count++;
    fs->sblock.used_block_count++;
    fs->sblock.inode_map[fs->current_dir].size++;

    char buffer[MAX_FILENAME_SIZE];
    memset(buffer, 0, MAX_FILENAME_SIZE);
    snprintf(buffer, MAX_FILENAME_SIZE, "%s", data[1]);
    minifs_append_data(fs, fs->current_dir, (const unsigned char*) buffer, MAX_FILENAME_SIZE);
    minifs_append_data(fs, fs->current_dir, (const unsigned char*) &inode_index, sizeof(uint32_t));
    minifs_update_superblock(fs);
}


void minifs_rmdir(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "rmdir command");
}


void minifs_touch(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "touch command");

    if (count < 2) {
        fprintf(stderr, "format: %s <filename>\n", data[0]);
        return;
    }
    if (strlen(data[1]) > MAX_FILENAME_SIZE) {
        fprintf(stderr, "filename is too long\n");
        return;
    }
    int32_t inode_index = minifs_find_free_inode(fs);
    int32_t block_index = minifs_find_free_block(fs);
    if (inode_index < 0) {
        fprintf(stderr, "Ran out of free inodes\n");
        return;
    }
    if (block_index < 0) {
        fprintf(stderr, "Ran out of free blocks\n");
        return;
    }

    fs->sblock.inode_map[inode_index].type = MINIFS_INODE_FILE;
    fs->sblock.inode_map[inode_index].root_block = block_index;
    fs->sblock.inode_map[inode_index].parent = -1;
    fs->sblock.inode_map[inode_index].size = 0;

    fs->sblock.block_map[block_index].size = 0;
    fs->sblock.block_map[block_index].next_block = -1;
    fs->sblock.block_map[block_index].type = MINIFS_BLOCK_USED;

    fs->sblock.used_inode_count++;
    fs->sblock.used_block_count++;
    fs->sblock.inode_map[fs->current_dir].size++;

    char buffer[MAX_FILENAME_SIZE];
    memset(buffer, 0, MAX_FILENAME_SIZE);
    snprintf(buffer, MAX_FILENAME_SIZE, "%s", data[1]);
    minifs_append_data(fs, fs->current_dir, (const unsigned char*) buffer, MAX_FILENAME_SIZE);
    minifs_append_data(fs, fs->current_dir, (const unsigned char*) &inode_index, sizeof(uint32_t));
    minifs_update_superblock(fs);
}


void minifs_rm(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "rm command");
}


void minifs_write(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "write command");
    if (count < 2) {
        fprintf(stderr, "format: %s <filename>", data[0]);
        return;
    }

    DirectoryMap *content = minifs_read_dir(fs, fs->current_dir);
    int32_t target_inode = -1;

    for (int index = 0; index < content->size; ++index) {
        if (fs->sblock.inode_map[content->inodes[index]].type == MINIFS_INODE_FILE) {
            if (strcmp(data[1], content->names[index]) == 0) {
                target_inode = content->inodes[index];
            }
        }
    }

    minifs_clear_dirmap(content);

    if (target_inode == -1) {
        fprintf(stderr, "No such file\n");
        return;
    }

    const char *input = readline("Enter data: ");
    minifs_append_data(fs, target_inode, (const unsigned char *) input, strlen(input));
    free((void*) input);

    fs->sblock.inode_map[target_inode].size += strlen(input);
    minifs_update_superblock(fs);
}


void minifs_read(Filesystem* fs, const char **data, int count) {
    debug(MINIFS_INFO "read command");
    if (count < 2) {
        fprintf(stderr, "format: %s <filename>", data[0]);
        return;
    }

    DirectoryMap *content = minifs_read_dir(fs, fs->current_dir);
    int32_t target_inode = -1;

    for (int index = 0; index < content->size; ++index) {
        if (fs->sblock.inode_map[content->inodes[index]].type == MINIFS_INODE_FILE) {
            if (strcmp(data[1], content->names[index]) == 0) {
                target_inode = content->inodes[index];
            }
        }
    }

    minifs_clear_dirmap(content);

    if (target_inode == -1) {
        fprintf(stderr, "No such file\n");
        return;
    }

    int32_t dsize = 0;
    const char *file_content = minifs_read_data(fs, target_inode, &dsize);
    for (int index = 0; index < dsize; ++index) {
        printf("%c", file_content[index]);
    }
    free((void*) file_content);

    printf("\n");
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


void minifs_debug(Filesystem *fs, const char **data, int count) {
    debug(MINIFS_INFO "debug command");
    printf("====== [Superblock] ======\n");
    printf("inode_count: %u\n", fs->sblock.inode_count);
    printf("block_count: %u\n", fs->sblock.block_count);
    printf("used_inode_count: %u\n", fs->sblock.used_inode_count);
    printf("used_block_count: %u\n", fs->sblock.used_block_count);
    printf("block_size: %u\n", fs->sblock.block_size);
    printf("===== [Inode map] ======\n");
    for (int index = 0; index < fs->sblock.inode_count; ++index) {
        char symbols[] = {'.', 'f', 'd'};
        printf("%c", symbols[fs->sblock.inode_map[index].type]);
    }
    printf("\n");
    printf("===== [Block map] ======\n");
    for (int index = 0; index < fs->sblock.block_count; ++index) {
        char symbols[] = {'.', '#'};
        printf("%c", symbols[fs->sblock.block_map[index].type]);
    }
    printf("\n");
    printf("===== [Inodes] ====== \n");
    for (int index = 0; index < fs->sblock.inode_count; ++index) {
        Inode inode = fs->sblock.inode_map[index];
        if (inode.type != MINIFS_INODE_EMPTY) {
            printf("Inode {size: %u, root_block %d, type: %d}\n", inode.size, inode.root_block, inode.type);
        }
    }
    printf("===== [Blocks] ====== \n");
    for (int index = 0; index < fs->sblock.block_count; ++index) {
        Block block = fs->sblock.block_map[index];
        if (block.type != MINIFS_BLOCK_EMPTY) {
            printf("Block {size: %u, next_block: %d}\n", block.size, block.next_block);
        }
    }
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