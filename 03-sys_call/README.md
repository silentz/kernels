## Syscall user storage

Linux 5.6.2 kernel patch. Adds three system calls for x86 and x86_64 platforms to store information in kernel.

### Build and setup

1. Download linux kernel 5.6.2 tarball:
```
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.6.2.tar.xz
```
2. Unpack tarball and add files from this repository to linux source folder or replace needed ones:
```
tar xJf linux-5.6.2.tar.xz
```
3. Install build tools:
```
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```
4. Build menuconfig and configure kernel:
```
make menuconfig
```
5. Build kernel:
```
make -j$(nproc)
```
6. Install kernel modules:
```
sudo make modules_install
```
7. Install kernel to system:
```
sudo make install
```
8. Reboot the system:
```
reboot
```

### Usage

After you compiled and ran patched kernel you can run following C program to check everything works fine:
```c
#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>


#define SYS_ADD_USER 439
#define SYS_GET_USER 440
#define SYS_DEL_USER 441


struct user_data {
    char name[1024];
    char surname[1024];
    char phone[1024];
    char email[1024];
} data;


int main() {
    strcpy(data.name, "test_name");
    strcpy(data.surname, "test_surname");
    strcpy(data.phone, "+123456789");
    strcpy(data.email, "test@test.com");

    int status;
    status = syscall(SYS_ADD_USER, &data);
    printf("sys_add_user: %d\n", status);

    struct user_data output;
    status = syscall(SYS_GET_USER, "test_surname", 12, &output);
    printf("sys_get_user: %d %s\n", status, output.name);

    status = syscall(SYS_DEL_USER, "test_surname", 12);
    printf("sys_del_user: %d\n", status);

    return 0;
}
```
