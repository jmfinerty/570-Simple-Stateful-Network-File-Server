// University of Kentucky CS570-001 "Modern Operating Systems", Fall 2021.
// Assignment 4: "Implement a Simple Stateful Network File Server"
// Due November 22nd, 2021 at 11:59PM.
// Work is original and by Jacob Finerty, student ID 912307438,
// with exception of provided sample code and files.
// Any sources referenced are cited in comments.
// Last modification before submission: 19 Nov 2021, 9:33PM

#ifndef DISKUTIL_H
#define DISKUTIL_H

// Disk
#define BLOCK_SIZE      512      // bytes per block, DEFINED IN ASSIGNMENT SPEC
#define DELIM_BUF_SIZE  2        // size buffer to read \ns into when reading vdisk in
#define FILE_SIZE       64       // blocks per file, DEFINED IN ASSIGNMENT SPEC
#define VDISK_CAPACITY  16777216 // size of virtual disk in bytes, 16mB, DEFINED IN ASSIGNMENT SPEC
#define VDISK_DELIM     "\n"     // delimiter to seperate lines in vdisk -- \n for fgets
#define VDISK_LOC       "vdisk"  // name of virtual disk file

// Files
#define DEFAULT_FILE_NAME "Fdq2RcMW" // name to assign to unallocated file space. randomly generated to reduce chance of file with this name
#define INIT_FILE_DESCRIP 9          // will start descriptors at 10 (9+=1)
#define MAX_FILE_NAME_LEN 20         // longest name a file can have -- FROM SSNFS.H, don't change

// Users
#define DEFAULT_USER_NAME "u48llq36" // name to assign unallocated space for users. randomly generated to reduce change of user with this name
#define MAX_NUM_USERS     16         // maximum number of users. should be power of 2 for division
#define MAX_USER_NAME_LEN 15         // maximum length of username, DEFINED IN ASSIGNMENT SPEC

// Derived
#define MAX_NUM_BLOCKS  (VDISK_CAPACITY / BLOCK_SIZE)    // maximum number of possible blocks with given disk space
#define MAX_USER_BLOCKS (MAX_NUM_BLOCKS / MAX_NUM_USERS) // maximum number of blocks a user can have if blocks evenly distributed between users
#define MAX_USER_FILES  (MAX_USER_BLOCKS / FILE_SIZE)    // maximum number of files a user can have if blocks evenly distributed between users
#define MAX_FT_SIZE     (MAX_USER_FILES * MAX_NUM_USERS) // maximum number of files that can exist in filetable
#define MAX_POINTER_POS (FILE_SIZE * BLOCK_SIZE)         // maximum position file pointer can have


struct _Block {                     // stores 1 block and data in block
    char data[BLOCK_SIZE+1];        // +1 because must be null terminated for fgets
}; typedef struct _Block Block;


struct _FileTableEntry {                          // stores an entry in the file table
    char ownerUserName[MAX_USER_NAME_LEN];        // name of user that file belongs to
    char fileName[MAX_FILE_NAME_LEN];             // name of the file
    int  fileDescriptor;                          // file descriptor value for file
    int  filePointerPos;                          // position of pointer in file
}; typedef struct _FileTableEntry FileTableEntry;

struct _FileTable {                      // file table, just a list of FileTableEntries
    FileTableEntry entries[MAX_FT_SIZE];
}; typedef struct _FileTable FileTable;


struct _FileInfo {                    // stores information about a file
    char name[MAX_FILE_NAME_LEN];     // stores name of file
    int  blocks[FILE_SIZE];           // stores the indices of a file's blocks in the Blocks struct
}; typedef struct _FileInfo FileInfo;

struct _User {                      // stores information about a user
    char name[MAX_USER_NAME_LEN];   // stores name of user
    FileInfo files[MAX_USER_FILES]; // stores information about each of users files
}; typedef struct _User User;

struct _DiskInfo {                    // stores information about users and the blocks they own
    char blocks[MAX_NUM_BLOCKS+1];    // stores if block at index is allocated or not (1/0). +1 because null term. for fgets
    User users[MAX_NUM_USERS];        // stores information about each user
}; typedef struct _DiskInfo DiskInfo;


extern FileTable* filetable;
extern DiskInfo di;
extern Block disk[MAX_NUM_BLOCKS];


// see comments in diskutil.c
FileTable* _initialize_file_table();
int _initialize_users_and_blocks();
int _read_update_from_vdisk();
int add_entry_to_file_table();
int add_file_to_di();
int add_user_to_di();
int drop_entry_from_file_table();
int drop_file_from_vdisk();
int write_update_to_vdisk();
int write_update_to_file_pointer_pos();

int load_or_initialize_virtual_disk();


#endif /* DISKUTIL_H */