#include <internal/fs/fs.h>
#include <internal/debug/debug.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


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

    struct SuperBlock sblock = {
        .inode_count = DEFAULT_INODE_COUNT,
        .block_count = DEFAULT_BLOCK_COUNT,
        .used_inode_count = 0,
        .used_block_count = 0,
        .block_size = DEFAULT_BLOCK_SIZE,
    };

    minifs_write_block(fd, (void*) &sblock, sizeof(struct SuperBlock), 0);

    Inode *inodes = (Inode*) malloc(sizeof(Inode) * DEFAULT_INODE_COUNT);
    memset(inodes, 0, DEFAULT_INODE_COUNT * sizeof(Inode));
    minifs_write_block(fd, inodes, DEFAULT_INODE_COUNT * sizeof(Inode), sizeof(SuperBlock));
    free(inodes);

    Block *blocks = (Block*) malloc(sizeof(Block) * DEFAULT_BLOCK_COUNT);
    memset(blocks, 0, DEFAULT_BLOCK_COUNT * sizeof(Block));
    minifs_write_block(fd, blocks, DEFAULT_BLOCK_SIZE * sizeof(Block), sizeof(SuperBlock) + DEFAULT_INODE_COUNT * sizeof(Inode));
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

    struct SuperBlock sblock;
    minifs_read_block(result.fd, (void*) &sblock, sizeof(struct SuperBlock), 0);

    sblock.inode_map = (Inode*) malloc(sblock.inode_count * sizeof(Inode));
    sblock.block_map = (Block*) malloc(sblock.block_count * sizeof(Block));
    minifs_read_block(result.fd, sblock.inode_map, sblock.inode_count * sizeof(Inode), sizeof(SuperBlock));
    minifs_read_block(result.fd, sblock.block_map, sblock.block_count * sizeof(Block), sizeof(SuperBlock) + sblock.inode_count * sizeof(Inode));

    result.sblock = sblock;
    return result;
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