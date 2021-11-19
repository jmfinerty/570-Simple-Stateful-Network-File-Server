#ifndef DISKUTIL_H
#define DISKUTIL_H

// Disk
#define BLOCK_SIZE      512      // bytes per block
#define DELIM_BUF_SIZE  2
#define FILE_SIZE       64       // blocks per file
#define VDISK_CAPACITY  16777216 // 16 mB
#define VDISK_DELIM     "\n"
#define VDISK_LOC       "vdisk"

// Files
#define DEFAULT_FILE_NAME "SOMEFILE"
#define INIT_FILE_DESCRIP 9          // will start descriptors at 10 (9+=1)
#define MAX_FILE_NAME_LEN 20

// Users
#define DEFAULT_USER_NAME "SOMEUSER"
#define MAX_NUM_USERS     16
#define MAX_USER_NAME_LEN 15

// Derived
#define MAX_NUM_BLOCKS  (VDISK_CAPACITY / BLOCK_SIZE)
#define MAX_USER_BLOCKS (MAX_NUM_BLOCKS / MAX_NUM_USERS)
#define MAX_USER_FILES  (MAX_USER_BLOCKS / FILE_SIZE)
#define MAX_FT_SIZE     (MAX_USER_FILES * MAX_NUM_USERS)
#define MAX_POINTER_POS (FILE_SIZE * BLOCK_SIZE)


struct _Block {
    char data[BLOCK_SIZE+1]; // +1 must be null terminated for fgets
}; typedef struct _Block Block;


struct _FileTableEntry {
    char ownerUserName[MAX_USER_NAME_LEN];
    char fileName[MAX_FILE_NAME_LEN];
    int  fileDescriptor;
    int  filePointerPos;
}; typedef struct _FileTableEntry FileTableEntry;

struct _FileTable {
    FileTableEntry entries[MAX_FT_SIZE];
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
    char blocks[MAX_NUM_BLOCKS+1];
    User users[MAX_NUM_USERS];
}; typedef struct _UsersBlocks UsersBlocks;


extern FileTable* filetable;
extern UsersBlocks ub;
extern Block blocks[MAX_NUM_BLOCKS];


FileTable* _initialize_file_table();
int _initialize_users_and_blocks();
int _read_update_from_vdisk();
int add_entry_to_file_table();
int add_file_to_usersblocks();
int add_user_to_usersblocks();
int drop_entry_from_file_table();
int drop_file_from_vdisk();
int write_update_to_vdisk();
int write_update_to_filetable();

int load_or_initialize_virtual_disk();


#endif /* DISKUTIL_H */