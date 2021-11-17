#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "diskutil.h"


FileTable* ft = NULL;
UsersBlocks ub;
Block blocks[MAX_NUM_BLOCKS];


FileTable* _initialize_file_table() {
    FileTable* table = (FileTable*) malloc(sizeof(FileTable)+1);

    // fill each file in table with default values
    for (int i = 0; i < MAX_FT_SIZE; i++) {
        strcpy(table->entry[i].ownerUserName, DEFAULT_USER_NAME);
        strcpy(table->entry[i].fileName, DEFAULT_FILE_NAME);
        table->entry[i].fileDescriptor = 0;
        table->entry[i].filePointerPos = 0;
    }

    return table;
}


// In a UsersBlocks,
// copies 0 into every block, DEFAULT_USER_NAME into every user's name,
// DEFAULT_FILE_NAME into every file name, and 0 into every file's blocks
int _initialize_users_and_blocks(UsersBlocks* usersblocks) {
    memset(usersblocks->blocks, '0', MAX_NUM_BLOCKS); // copy 0 into every block
    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        memset(blocks[block].data, ' ', BLOCK_SIZE);
    }

    for (int user = 0; user < MAX_NUM_USERS; user++) {
        strcpy(usersblocks->users[user].name, DEFAULT_USER_NAME); // copy default
        for (int file = 0; file < MAX_USER_FILES; file++) {
            strcpy(usersblocks->users[user].files[file].name, DEFAULT_FILE_NAME);
            for (int block = 0; block < FILE_SIZE; block++) {
                usersblocks->users[user].files[file].blocks[block] = 0;
            }
        }
    }

    return 1;
}


int _read_update_from_vdisk() {
    FILE* vdisk = fopen(VDISK_LOC, "r");

    for (int user = 0; user < MAX_NUM_USERS; user++) {
        fgets(ub.users[user].name, MAX_USER_NAME_LEN, vdisk);
        for (int file = 0; file < MAX_USER_FILES; file++) {
            fgets(ub.users[user].files[file].name, MAX_FILE_NAME_LEN, vdisk);
            for (int block = 0; block < FILE_SIZE; block++) {
                fscanf(vdisk, "%d ", &ub.users[user].files[file].blocks[block]);
            }
        }
    }

    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        fgets(blocks[block].data, BLOCK_SIZE, vdisk);
    }

    fclose(vdisk);
    return 1;
}


int _write_update_to_vdisk() {
    FILE* vdisk = fopen(VDISK_LOC, "w");
    //https://www.tutorialspoint.com/cprogramming/c_file_io.htm

    fputs(ub.blocks, vdisk);

    for (int user = 0; user < MAX_NUM_USERS; user++) {
        fputs(ub.users[user].name, vdisk);
        fputs(VDISK_DELIM, vdisk);
        for (int file = 0; file < MAX_USER_FILES; file++) {
            fputs(ub.users[user].files[file].name, vdisk);
            fputs(VDISK_DELIM, vdisk);
            for (int block = 0; block < FILE_SIZE; block++) {
                fputs(VDISK_DELIM, vdisk);
                //fputs(ub.users[user].files[file].blocks[block], vdisk);
                fprintf(vdisk, "%d ", ub.users[user].files[file].blocks[block]);
            }
        }
        fputs(VDISK_DELIM, vdisk);
    }

    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        fputs(blocks[block].data, vdisk);
        fputs(VDISK_DELIM, vdisk);
    }

    fclose(vdisk);
    return 1;
}


int initialize_virtual_disk() {
    if (ft == NULL) {
        ft = _initialize_file_table();
    }

    // check if vdisk exists. F_OK = check existence
    // https://linux.die.net/man/2/access
    if (access(VDISK_LOC, F_OK)) {
        _initialize_users_and_blocks(&ub);
    } else {
        FILE* vdisk = fopen(VDISK_LOC, "r");
    }

    return 1;

}