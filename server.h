
#define BLOCK_SIZE     512      // bytes per block
#define VDISK_CAPACITY 16777216 // 16 mB
#define MAX_NUM_BLOCKS 32768    // max capacity / size of block

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
    FileTableEntry table[MAX_FT_SIZE];
}; typedef struct _FileTable FileTable;