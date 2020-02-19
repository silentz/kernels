## Storage kernel module

A linux kernel module that keeps all your contacts in one place.

### Build and setup

1. First of all, build a module using:
```
make
```
2. Next you can run it using following command:
```
sudo ./run.sh
```
3. After all you can turn the module off using following command:
```
sudo ./stop.sh
```
4. Finally, you can clean up build files using:
```
make clean
```

### Usage

After you compiled and ran module, you can do following actions:
1. Add contact to storage:
```
echo 'add Name Surname 42 +12345678901 test@email.com' > /dev/storage
```
2. Get contact from storage:
```
echo 'get Surname' > /dev/storage
cat /dev/storage
```
3. Delete contact from storage:
```
echo 'delete Surname' > /dev/storage
```
