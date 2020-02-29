#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>


#define DEFAULT_INODE_COUNT 1024
#define DEFAULT_BLOCK_COUNT 1024
#define DEFAULT_BLOCK_SIZE  1024


// superblock of minifs
struct SuperBlock {
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t used_inode_count;
    uint32_t used_block_count;
    uint32_t block_size;
    void *inode_bitmap;
    void *block_bitmap;
};


// types of inode
enum InodeType {
    MINIFS_FILE,
    MINIFS_DIRECTOTY
};


// block to store information and meta
struct Block {
    void *next;
    void *data;
};


// inode srtuct
struct Inode {
    uint32_t file_size;
    struct Block *root;
    enum InodeType type;
};


// filesystem controller block
struct Filesystem {
    struct SuperBlock sblock;
    int fd;
};


void minifs_init(const char *);
struct Filesystem minifs_open(const char *);
bool check_exists(const char *);

// fd, data, size, offset
void minifs_write_block(int, void *, uint32_t, uint32_t);
void minifs_read_block(int, void*, uint32_t, uint32_t);

#endif