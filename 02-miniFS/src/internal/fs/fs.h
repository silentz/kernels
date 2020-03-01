#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>


#define DEFAULT_INODE_COUNT 1024
#define DEFAULT_BLOCK_COUNT 1024
#define DEFAULT_BLOCK_SIZE  1024
#define MAX_FILENAME_SIZE   28

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
    enum BlockType {
        MINIFS_BLOCK_EMPTY = 0,
        MINIFS_BLOCK_USED = 1
    } type;
} Block;


// inode srtuct
typedef struct Inode {
    uint32_t size;      // size of file or count of objects in dir
    int32_t root_block;
    int32_t parent;
    enum InodeType {
        MINIFS_INODE_EMPTY = 0,
        MINIFS_INODE_FILE = 1,
        MINIFS_INODE_DIRECTORY = 2
    } type;
} Inode;


// filesystem controller block
typedef struct Filesystem {
    struct SuperBlock sblock;
    uint32_t current_dir;       // inode id
    int fd;
} Filesystem;


typedef struct DirectoryMap {
    uint32_t size;
    char **names;
    uint32_t *inodes;
} DirectoryMap;


void minifs_init(const char *);
struct Filesystem minifs_open(const char *);
bool check_exists(const char *);


// fd, data, size, offset
void minifs_write_block(int, void *, uint32_t, uint32_t);
void minifs_read_block(int, void*, uint32_t, uint32_t);


uint32_t minifs_block_head_offset(Filesystem*, uint32_t);
uint32_t minifs_block_body_offset(Filesystem*, uint32_t);
uint32_t minifs_inode_offset(Filesystem*, uint32_t);

DirectoryMap *minifs_read_dir(Filesystem*, uint32_t);
void minifs_clear_dirmap(DirectoryMap*);


int32_t minifs_find_free_inode(Filesystem*);
int32_t minifs_find_free_block(Filesystem*);
void minifs_update_superblock(Filesystem*);
void minifs_append_data(Filesystem*, uint32_t, const unsigned char *, uint32_t);
const char* minifs_read_data(Filesystem*, int32_t, int32_t*);

#endif