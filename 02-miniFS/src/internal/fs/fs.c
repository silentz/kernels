#include <internal/fs/fs.h>
#include <internal/debug/debug.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool check_exists(const char *filename) {
    struct stat info;
    int code = stat(filename, &info);
    if (code == -1) {
        return false;
    }
    if (info.st_size < sizeof(struct SuperBlock)) {
        return false;
    }
    return true;
}


void minifs_init(const char *filename) {
    int fd = open(filename, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);

    if (fd < 0) {
        debug(MINIFS_ERR "error while opening file: %s", filename);
        exit(-1);
    }

    // init superblock

    struct SuperBlock sblock = {
        .inode_count = DEFAULT_INODE_COUNT,
        .block_count = DEFAULT_BLOCK_COUNT,
        .used_inode_count = 0,
        .used_block_count = 0,
        .block_size = DEFAULT_BLOCK_SIZE,
    };

    Inode *inodes = (Inode*) malloc(sizeof(Inode) * DEFAULT_INODE_COUNT);
    memset(inodes, 0, DEFAULT_INODE_COUNT * sizeof(Inode));

    Block *blocks = (Block*) malloc(sizeof(Block) * DEFAULT_BLOCK_COUNT);
    memset(blocks, 0, DEFAULT_BLOCK_COUNT * sizeof(Block));

    // init root dir

    inodes[0].root_block = 0;
    inodes[0].size = 0;
    inodes[0].type = MINIFS_INODE_DIRECTORY;

    blocks[0].next_block = -1;
    blocks[0].size = 0;
    blocks[0].type = MINIFS_BLOCK_USED;

    // write changes

    minifs_write_block(fd, (void*) &sblock, sizeof(struct SuperBlock), 0);
    minifs_write_block(fd, inodes, DEFAULT_INODE_COUNT * sizeof(Inode), sizeof(SuperBlock));
    minifs_write_block(fd, blocks, DEFAULT_BLOCK_SIZE * sizeof(Block), sizeof(SuperBlock) + DEFAULT_INODE_COUNT * sizeof(Inode));
    free(inodes);
    free(blocks);

    close(fd);
}


struct Filesystem minifs_open(const char *filename) {
    struct Filesystem result;

    result.fd = open(filename, O_RDWR);
    if (result.fd < 0) {
        debug(MINIFS_ERR "cannot open filesystem: %s", filename);
        exit(-1);
    }

    // read superblock

    struct SuperBlock sblock;
    minifs_read_block(result.fd, (void*) &sblock, sizeof(struct SuperBlock), 0);

    // reading inode and block map

    sblock.inode_map = (Inode*) malloc(sblock.inode_count * sizeof(Inode));
    sblock.block_map = (Block*) malloc(sblock.block_count * sizeof(Block));
    minifs_read_block(result.fd, sblock.inode_map, sblock.inode_count * sizeof(Inode), sizeof(SuperBlock));
    minifs_read_block(result.fd, sblock.block_map, sblock.block_count * sizeof(Block), sizeof(SuperBlock) + sblock.inode_count * sizeof(Inode));

    // setup filesystem

    result.sblock = sblock;
    result.current_dir = 0;

    return result;
}


uint32_t minifs_block_head_offset(Filesystem *fs, uint32_t index) {
    uint32_t result = 0;
    result += sizeof(SuperBlock);
    result += fs->sblock.inode_count * sizeof(Inode);
    result += index * sizeof(Block);
    return result;
}


uint32_t minifs_block_body_offset(Filesystem *fs, uint32_t index) {
    uint32_t result = 0;
    result += sizeof(SuperBlock);
    result += fs->sblock.inode_count * sizeof(Inode);
    result += fs->sblock.block_count * sizeof(Block);
    result += fs->sblock.block_size * index;
    return result;
}


uint32_t minifs_inode_offset(Filesystem *fs, uint32_t index) {
    uint32_t result = 0;
    result += sizeof(SuperBlock);
    result += index * sizeof(Inode);
    return result;
}


DirectoryMap *minifs_read_dir(Filesystem *fs, uint32_t dir_inode) {
    // init map and read inode
    DirectoryMap *result = (DirectoryMap*) malloc(sizeof(DirectoryMap));
    Inode inode = fs->sblock.inode_map[dir_inode];

    // setup array
    result->size = 0; // used as a counter in loop
    result->names = (char**) malloc(sizeof(char*) * inode.size);
    result->inodes = (uint32_t*) malloc(sizeof(uint32_t) * inode.size);

    // go through all blocks
    int32_t current_block = inode.root_block;
    while (current_block >= 0) {
        Block block = fs->sblock.block_map[current_block];  // current block meta
        uint32_t count = block.size / (MAX_FILENAME_SIZE + sizeof(uint32_t));   // count of entries in block
        uint32_t current_offset = minifs_block_body_offset(fs, current_block);  // offset to current physical block

        for (int index = 0; index < count; ++index) {
            char *name = (char*) malloc(MAX_FILENAME_SIZE);
            uint32_t inode_id;
            uint32_t local_offset = current_offset + index * (MAX_FILENAME_SIZE + sizeof(uint32_t));
            minifs_read_block(fs->fd, name, MAX_FILENAME_SIZE, local_offset);
            minifs_read_block(fs->fd, &inode_id, sizeof(uint32_t), local_offset + MAX_FILENAME_SIZE);

            // save info
            result->names[result->size] = name;
            result->inodes[result->size] = inode_id;
            ++result->size;
        }

        current_block = block.next_block;
    }

    return result;
}


void minifs_clear_dirmap(DirectoryMap *map) {
    for (uint32_t index = 0; index < map->size; ++index) {
        free(map->names[index]);
    }
    free(map->names);
    free(map->inodes);
    free(map);
}


int32_t minifs_find_free_inode(Filesystem *fs) {
    if (fs->sblock.used_inode_count >= fs->sblock.inode_count) {
        return -1;
    }
    for (uint32_t index = 0; index < fs->sblock.inode_count; ++index) {
        if (fs->sblock.inode_map[index].type == MINIFS_INODE_EMPTY) {
            return index;
        }
    }
    return -1;
}


int32_t minifs_find_free_block(Filesystem *fs) {
    if (fs->sblock.used_block_count >= fs->sblock.block_count) {
        return -1;
    }
    for (uint32_t index = 0; index < fs->sblock.block_count; ++index) {
        if (fs->sblock.block_map[index].type == MINIFS_BLOCK_EMPTY) {
            return index;
        }
    }
    return -1;
}


void minifs_update_superblock(Filesystem *fs) {
    minifs_write_block(fs->fd, &fs->sblock, sizeof(SuperBlock), 0);
    for (uint32_t index = 0; index < fs->sblock.inode_count; ++index) {
        uint32_t offset = minifs_inode_offset(fs, index);
        minifs_write_block(fs->fd, &fs->sblock.inode_map[index], sizeof(Inode), offset);
    }
    for (uint32_t index = 0; index < fs->sblock.block_count; ++index) {
        uint32_t offset = minifs_block_head_offset(fs, index);
        minifs_write_block(fs->fd, &fs->sblock.block_map[index], sizeof(Block), offset);
    }
}


void minifs_append_dir(Filesystem *fs, uint32_t dir_inode, const char *name, uint32_t inode) {
    uint32_t current_block_id = fs->sblock.inode_map[dir_inode].root_block;
    Block current_block = fs->sblock.block_map[current_block_id];
    while (current_block_id >= 0) {
        if (current_block.size + (sizeof(uint32_t) + MAX_FILENAME_SIZE) > fs->sblock.block_size) {
            if (current_block.next_block < 0) {
                int32_t free_block = minifs_find_free_block(fs);
                if (free_block < 0) {
                    fprintf(stderr, "Ran out of free blocks\n");
                    return;
                }
                fs->sblock.block_map[current_block_id].next_block = free_block;
                current_block = fs->sblock.block_map[current_block_id];
            }
            current_block_id = current_block.next_block;
            current_block = fs->sblock.block_map[current_block_id];
            continue;
        }
        uint32_t offset = minifs_block_body_offset(fs, current_block_id);
        offset += current_block.size;
        minifs_write_block(fs->fd, (void *) name, MAX_FILENAME_SIZE, offset);
        minifs_write_block(fs->fd, &inode, sizeof(uint32_t), offset + MAX_FILENAME_SIZE);
        fs->sblock.block_map[current_block_id].size += MAX_FILENAME_SIZE + sizeof(uint32_t);
        break;
    }
}


void minifs_read_block(int fd, void *data, uint32_t size, uint32_t offset) {
    lseek(fd, offset, SEEK_SET);
    uint32_t read_size = 0;
    while (read_size < size) {
        uint32_t status = read(fd, data + read_size, size - read_size);
        if (status <= 0) {
            debug(MINIFS_ERR "read error");
            exit(-1);
        }
        read_size += status;
    }
}


void minifs_write_block(int fd, void *data, uint32_t size, uint32_t offset) {
    lseek(fd, offset, SEEK_SET);
    uint32_t write_size = 0;
    while (write_size < size) {
        uint32_t status = write(fd, data + write_size, size - write_size);
        if (status <= 0) {
            debug(MINIFS_ERR "write error");
            exit(-1);
        }
        write_size += status;
    }
}