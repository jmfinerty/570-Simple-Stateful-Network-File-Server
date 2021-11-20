# 570-Simple-Stateful-Network-File-Server

See `hw4.pdf` for full assignment specification.

## Building Executables
From the code's directory, run `make`. Make will run `rpcgen`. Note that use of `make clean` will remove the virtual disk and thus the stateful storage.

If errors like `"Cannot register service..."` occur, check with `rpcinfo` that the RPC portmapper service is running. If this does not work, install `portmap` and run `rpcbind`.

## Running Executables
The server executable can simply be run with `./server`. The client executable can be run with `./client host`. If the server is running locally, this can just be `./client localhost`.

## Contents

#### services.c

Stores the functions responsible for the primary services of the server.

#### diskutil.c, diskutil.h

Stores functions and constants used in resource management. I.e., disk and file table management. Used only by server.c.

#### serverutil.c, serverutil.h

Stores functions and constants used in resource reading. I.e., getter functions to retrieve information from the disk and file table.

#### client.c

Stores the functions responsible for the primary services of the client.

#### ssnfs.x

Defines the various interfaces exported by the server's services. Used by `rpcgen`. Provided by the assignment specifications. This file was not allowed to be modified at all, and has not been modified.

## Services
Each of these services will print output to the terminal in both the server and the client.

Additionally, each of these services (to the best of my knowledge) follow their description in the assignment specification. There are brief descriptions of each below, followed by their more-detailed specification.

#### Open
Marks a specified file name in the user's directory as open.
> Opens the file with the given name in the user’s directory and returns a file descriptor (a positive integer) to the client. The file descriptor is used in subsequent reads and writes. If there is no file with the given file name in the user’s directory, it creates a file with that name and returns the file descriptor. If it cannot open a file for some reason (for example, no space on the disk to create a new file, or file table at the server has grown large. You can assume size of file table to be 20.) it returns -1 as file descriptor. Each user is assigned his/her own directory with his/her login name as the name of the directory. The login name of a user can be obtained from the password file (using the command getpwuid(getuid())->pw name). A newly created file is allocated 64 blocks. File size is fixed (i.e., files cannot grow dynamically).

#### Write
Writes a specified number of bytes from a given buffer to the user's file matching the file descriptor. Spec:
> writes the specified number of bytes from the buffer to the file represented by the file descriptor from the current position and advances the file pointer the number of bytes written. Uses variable length buffer. Returns appropriate error message if write fails.

#### Read
Reads a specified number of bytes from a file with a given descriptor that belongs to the user. These bytes are read into a buffer, which is returned to the client. The file pointer is also advanced. Spec:
> reads the specified number of bytes from the current position and returns it to the client and advances the file pointer by the number of bytes read. Uses variable length buffer. Returns appropriate error message if trying to read past the end of file, file descriptor passed was not correct, etc..

#### List
Simply lists the names of each of the existing files owned by the user. This is intentionally based off the disk, not the file table, so closed files still appear but deleted ones do not. Spec:
> lists the names of all files in the user’s directory.

#### Delete
Deletes a file. The file is closed as described below. Then, each of the bytes in each of its allocated blocks are freed. That is, the blocks are not actually cleared. Rather, they are marked as available, and will be overwritten by the next file written to that slot. Spec:
> deletes the specified file.

#### Close
Given a file descriptor, closes that file. The file is removed from the file table, but not from the disk. Any stored information about its pointer, its name, its owner, and its descriptor value will be lost. Spec:
> closes the file with given file descriptor. After a file is closed, the user should not be able to read from the file or write to the file.

## Virtual Disk Format
The virtual disk has a header which stores (in order):
1. Index of allocated blocks
2. Directory names
3. File names in directory
4. Block indices in file
5. Repeat 2-4 until end of file table.

At the end of this header, the data is stored.

Be cautious of opening the disk in a text editor, as it stores 16777216 bytes of data, plus a header and delimiters. This usually crashes VSCode on my machine.

## Additional Notes
- Each of these files, services, and functions is fully commented.
- File descriptors are always unique. If a file is opened, then closed, then opened, the file descriptor returned by the second open will be different from that returned by the first. However, they will both still represent the same file and contents. File content location is based on file name.
- A warning will be printed in the server if it appears that the user read from a file without first resetting the pointer. Files are required to be 64 blocks of 512 bytes. Bytes not written to must exist in the file, but be empty. I represent this with space characters. So, if a user opens, then writes, then reads, the read will begin at the file pointer produced after the write, and will thus read in spaces. This is possibly desired by the user, so it is allowed, but the aforementioned warning will be printed.
- The specification states that we ***can*** assume file table size to be 20. I have chosen not to, and have instead set it to be the maximum number of files a user may have, multiplied by the maximum number of users.
- The user's home directory is their username.

## Additional Specifications
- > You can think of the virtual disk as a sequence of blocks, each block containing 512 bytes.
- > You can also assume that the capacity of virtual disk is 16MB.
- > Each user should be assigned by the server a home directory.
- > Users do not have the ability to create subdirectories within their home directory.
