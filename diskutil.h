#ifndef DISKUTIL_H
#define DISKUTIL_H

// Disk
#define VDISK_LOC      "vdisk"
#define VDISK_CAPACITY 16777216 // 16 mB
#define BLOCK_SIZE     512      // bytes per block
#define FILE_SIZE      64       // blocks per file

// Files
#define DEFAULT_FILE_NAME "SOMEFILE"
#define MAX_FILE_NAME_LEN 20

// Users
#define DEFAULT_USER_NAME "SOMEUSER"
#define MAX_USER_NAME_LEN 15
#define MAX_NUM_USERS     16

// Derived
#define MAX_NUM_BLOCKS  (VDISK_CAPACITY / BLOCK_SIZE)
#define MAX_FT_SIZE     (MAX_NUM_BLOCKS / FILE_SIZE)
#define MAX_USER_BLOCKS (MAX_NUM_BLOCKS / MAX_NUM_USERS)
#define MAX_USER_FILES  (MAX_NUM_BLOCKS / MAX_NUM_USERS)


struct _FileTableEntry {
    char ownerUserName[MAX_USER_NAME_LEN];
    char fileName[MAX_FILE_NAME_LEN];
    int  fileDescriptor;
    int  filePointerPos;
}; typedef struct _FileTableEntry FileTableEntry;

struct _FileTable {
    FileTableEntry entry[MAX_FT_SIZE];
}; typedef struct _FileTable FileTable;

struct _FileInfo {
    char name[MAX_FILE_NAME_LEN];
    int  blocks[FILE_SIZE];
}; typedef struct _FileInfo FileInfo;

struct _User {
    char name[MAX_USER_NAME_LEN];
    FileInfo files[MAX_USER_FILES];
}; typedef struct _User User;

struct _UsersBlocks {
    char blocks[MAX_USER_BLOCKS];
    User users[MAX_NUM_USERS];
}; typedef struct _UsersBlocks UsersBlocks;


FileTable* _initialize_file_table();
int initialize_virtual_disk();


#endif /* DISKUTIL_H */