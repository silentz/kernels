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

    unsigned char *bits = (unsigned char*) malloc(DEFAULT_INODE_COUNT);
    memset(bits, 0, DEFAULT_INODE_COUNT);
    minifs_write_block(fd, (void*) bits, DEFAULT_INODE_COUNT, sizeof(struct SuperBlock));
    free(bits);

    bits = (unsigned char*) malloc(DEFAULT_BLOCK_COUNT);
    memset(bits, 0, DEFAULT_BLOCK_COUNT);
    minifs_write_block(fd, (void*) bits, DEFAULT_BLOCK_SIZE, sizeof(struct SuperBlock) + DEFAULT_INODE_COUNT);
    free(bits);

    close(fd);
}


struct Filesystem minifs_open(const char *filename) {
    struct Filesystem result;
    return result;
}


void minifs_write_block(int fd, void *data, uint32_t size, uint32_t offset) {
    lseek(fd, offset, SEEK_SET);
    uint32_t write_size = 0;
    while (write_size < size) {
        write_size += write(fd, data + write_size, size - write_size);
    }
}