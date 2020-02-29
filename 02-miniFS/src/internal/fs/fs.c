#include <internal/fs/fs.h>
#include <sys/stat.h>

bool check_exists(const char *filename) {
    struct stat info;
    int code = stat(filename, &info);
    if (code == -1) {
        return false;
    }
    
    return true;
}