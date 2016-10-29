# A simple local chat room in Linux Ubuntu
Basic program for communication bewtween process.
Those process are link to a shared memory, every single of message will pass by the shared memory.

## To compile the program:
```shell
make all
```

## Server Usage:
```shell
./server
```

## Client Usage:
```shell
./client "id" "name"
```
Note that Server id is zero
so zero id and repeated id is illegal.
