## MiniFS

Small filesystem which uses same disc structure as ext family filesystems.
Can store directories and files and uses recursive directory structure.

### Build and setup

1. First of all, you need cmake to compile project:
```
mkdir build
cd build
cmake ..
```
2. Next you can run make command. It will compile project binary:
```
make
```

### Usage

After you compile binary, you can run it using following syntax:
```
./minifs filename
```
where `filename` is place to store filesystem data. 

Inside command repl of minifs you can use following commands:
1. Create directory:
```
mkdir data
```
2. Create file:
```
touch filename
```
3. Change directory:
```
cd data
```
4. List files inside current directory:
```
ls
```
5. Write data to file:
```
write filename
> Add some text: <your input here>
```
6. Read file:
```
read filename
```
7. Remove file:
```
rm filename
```
8. Remove directory:
```
rmdir data
```
9. Exit minifs:
```
exit
```
10. Print filesystem superblock and inode/block map:
```
debug
```
