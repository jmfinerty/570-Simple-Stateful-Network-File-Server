#ifndef DISKUTIL_H
#define DISKUTIL_H

#define DEFAULT_FILE_NAME "SOMEFILE"
#define DEFAULT_USER_NAME "SOMEUSER"

#define VDISK_LOC "vdisk"

#define MAX_FT_SIZE       200
#define MAX_FILE_NAME_LEN 20
#define MAX_USER_NAME_LEN 15


struct _FileTableEntry {
    char ownerUserName[MAX_USER_NAME_LEN];
    char fileName[MAX_FILE_NAME_LEN];
    int  fileDescriptor;
    int  filePointerPos;
}; typedef struct _FileTableEntry FileTableEntry;

struct _FileTable {
    FileTableEntry entry[MAX_FT_SIZE];
}; typedef struct _FileTable FileTable;


FileTable* _initialize_file_table();
int initialize_virtual_disk();


#endif /* DISKUTIL_H */