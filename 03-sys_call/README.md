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
```

```
