#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>


// superblock of minifs
struct SuperBlock {
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t free_inode_count;
    uint32_t free_block_coint;
    uint32_t block_size;
    void *inode_bitmap;
    void *block_bitmap;
};


// types of inode
enum InodeType {
    FILE,
    DIRECTOTY
};


// inode srtuct
struct Inode {
    uint32_t file_size;
    struct Block *root;
    enum InodeType type;
};


// block to store information and meta
struct Block {
    void *next;
    void *data;
};


// filesystem controller block
struct Filesystem {
    struct SuperBlock sblock;
    int fd;
};


struct Filesystem init(const char *);
struct Filesystem open(const char *);
bool check_exists(const char *);

#endif