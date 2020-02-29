#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>


#define DEFAULT_INODE_COUNT 1024
#define DEFAULT_BLOCK_COUNT 1024
#define DEFAULT_BLOCK_SIZE  1024


struct Inode;
struct Block;


// superblock of minifs
typedef struct SuperBlock {
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t used_inode_count;
    uint32_t used_block_count;
    uint32_t block_size;
    struct Inode *inode_map;
    struct Block *block_map;
} SuperBlock;


// block to store information and meta
typedef struct Block {
    int32_t next_block;
    uint32_t size;
} Block;


// inode srtuct
typedef struct Inode {
    uint32_t file_size;
    int32_t root_block;
    enum InodeType {
        MINIFS_FILE,
        MINIFS_DIRECTOTY
    } type;
} Inode;


// filesystem controller block
typedef struct Filesystem {
    struct SuperBlock sblock;
    int fd;
} Filesystem;


void minifs_init(const char *);
struct Filesystem minifs_open(const char *);
bool check_exists(const char *);


// fd, data, size, offset
void minifs_write_block(int, void *, uint32_t, uint32_t);
void minifs_read_block(int, void*, uint32_t, uint32_t);

#endif