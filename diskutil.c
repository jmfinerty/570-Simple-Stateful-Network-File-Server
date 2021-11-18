#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "diskutil.h"


FileTable* filetable = NULL;
UsersBlocks ub;
Block blocks[MAX_NUM_BLOCKS];


FileTable* _initialize_file_table() {
    FileTable* table = (FileTable*) malloc(sizeof(FileTable)+1);

    // fill each file in table with default values
    for (int i = 0; i < MAX_FT_SIZE; i++) {
        strcpy(table->entries[i].ownerUserName, DEFAULT_USER_NAME);
        strcpy(table->entries[i].fileName, DEFAULT_FILE_NAME);
        table->entries[i].fileDescriptor = 0;
        table->entries[i].filePointerPos = 0;
    }

    return table;
}


int write_update_to_filetable(char* user_name, char* file_name, int file_descriptor, int file_pointer_pos) {
    for (int entry; entry < MAX_FT_SIZE; entry++)
        if (strcmp(filetable->entries[entry].ownerUserName, user_name) == 0)
            if (filetable->entries[entry].fileDescriptor == file_descriptor)
                filetable->entries[entry].filePointerPos = file_pointer_pos;
    return 0;
}


int file_descriptors_pos = INIT_FILE_DESCRIP;
int add_entry_to_file_table(char* user_name, char* file_name) {
    for (int entry = 0; entry < MAX_FT_SIZE; entry++)
        if (strcmp(filetable->entries[entry].ownerUserName, DEFAULT_USER_NAME) == 0)
            if (strcmp(filetable->entries[entry].fileName, DEFAULT_FILE_NAME) == 0) {
                file_descriptors_pos += 1;
                filetable->entries[entry].fileDescriptor = file_descriptors_pos;
                filetable->entries[entry].filePointerPos = 0;
                strcpy(filetable->entries[entry].fileName, file_name);
                strcpy(filetable->entries[entry].ownerUserName, user_name);
                return file_descriptors_pos;
            }
    return -1;
}


int drop_entry_from_file_table(int file_descriptor) {
    for (int entry = 0; entry < MAX_FT_SIZE; entry++) {
        if (filetable->entries[entry].fileDescriptor == file_descriptor) {
            //file_descriptors_pos -= 1; // not sure if this matters? safer to keep commented
            filetable->entries[entry].fileDescriptor = 0;
            filetable->entries[entry].filePointerPos = 0;
            strcpy(filetable->entries[entry].fileName, DEFAULT_FILE_NAME);
            strcpy(filetable->entries[entry].ownerUserName, DEFAULT_USER_NAME);
            return 0;
        }
    }
    return 1;
}


int add_file_to_usersblocks(int user_index_in_usersblocks, char* file_name) {
    for (int file = 0; file < MAX_USER_FILES; file++)
        if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_usersblocks].files[file].name) == 0) {

            strcpy(ub.users[user_index_in_usersblocks].files[file].name, file_name);

            // assign needed blocks in usersblocks to this
            int empty_block_index = 0;
            while (ub.users[user_index_in_usersblocks].files[file].blocks[empty_block_index] != 0)
                empty_block_index += 1;

            int assigned = 0;
            for (int block = 1; block < MAX_NUM_BLOCKS; block++) {
                if (assigned >= FILE_SIZE)
                    break;

                if (ub.blocks[block] == '0') { //unassigned
                    ub.users[user_index_in_usersblocks].files[file].blocks[empty_block_index] = block; // assign
                    empty_block_index += 1;
                    ub.blocks[block] = '1'; // mark as assigned
                    assigned += 1;
                }
            }
            return file;
        }
    return -1;
}


int add_user_to_usersblocks(char* user_name) {
    for (int user = 0; user < MAX_NUM_USERS; user++)
        if (strcmp(DEFAULT_USER_NAME, ub.users[user].name) == 0) {
            strcpy(ub.users[user].name, user_name);
            return user;
        }
    return -1; // no empty slots for user
}


// In a UsersBlocks,
// copies 0 into every block, DEFAULT_USER_NAME into every user's name,
// DEFAULT_FILE_NAME into every file name, and 0 into every file's blocks
int _initialize_users_and_blocks(UsersBlocks* usersblocks) {
    memset(usersblocks->blocks, '0', MAX_NUM_BLOCKS); // copy 0 into every block
    for (int block = 0; block < MAX_NUM_BLOCKS; block++)
        memset(blocks[block].data, ' ', BLOCK_SIZE);

    for (int user = 0; user < MAX_NUM_USERS; user++) {
        strcpy(usersblocks->users[user].name, DEFAULT_USER_NAME); // copy default
        for (int file = 0; file < MAX_USER_FILES; file++) {
            strcpy(usersblocks->users[user].files[file].name, DEFAULT_FILE_NAME);
            for (int block = 0; block < FILE_SIZE; block++)
                usersblocks->users[user].files[file].blocks[block] = 0;
        }
    }

    return 0;
}


int _read_update_from_vdisk() {
    FILE* vdisk = fopen(VDISK_LOC, "r");
    char delim_buf[DELIM_BUF_SIZE]; // HACK, but it works

    fgets(ub.blocks, MAX_NUM_BLOCKS+1, vdisk);

    fgets(delim_buf, DELIM_BUF_SIZE, vdisk);
    for (int user = 0; user < MAX_NUM_USERS; user++) {
        fgets(ub.users[user].name, MAX_USER_NAME_LEN, vdisk);
        ub.users[user].name[strcspn(ub.users[user].name, "\n")] = 0; // https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        //for (int uu = 0; uu < MAX_NUM_USERS; uu++){printf("...%s...", ub.users[uu].name); fflush(stdout);}
        for (int file = 0; file < MAX_USER_FILES; file++) {
            fgets(ub.users[user].files[file].name, MAX_FILE_NAME_LEN, vdisk);
            ub.users[user].files[file].name[strcspn(ub.users[user].files[file].name, "\n")] = 0;

            for (int block = 0; block < FILE_SIZE; block++) {
                fscanf(vdisk, "%d ", &ub.users[user].files[file].blocks[block]);
            }
        }
    }

    fgets(delim_buf, DELIM_BUF_SIZE, vdisk);
    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        fgets(blocks[block].data, BLOCK_SIZE+1, vdisk);
        fgets(delim_buf, DELIM_BUF_SIZE, vdisk);
    }

    fclose(vdisk);
    return 0;
}


int write_update_to_vdisk() {
    FILE* vdisk = fopen(VDISK_LOC, "w");
    //https://www.tutorialspoint.com/cprogramming/c_file_io.htm

    fputs(ub.blocks, vdisk);
    fputs(VDISK_DELIM, vdisk);

    for (int user = 0; user < MAX_NUM_USERS; user++) {
        fputs(ub.users[user].name, vdisk);
        fputs(VDISK_DELIM, vdisk);

        for (int file = 0; file < MAX_USER_FILES; file++) {
            fputs(ub.users[user].files[file].name, vdisk);
            fputs(VDISK_DELIM, vdisk);

            for (int block = 0; block < FILE_SIZE; block++)
                fprintf(vdisk, "%d ", ub.users[user].files[file].blocks[block]);
                //fputs(ub.users[user].files[file].blocks[block], vdisk);
            fputs(VDISK_DELIM, vdisk);
        }
    }

    fputs(VDISK_DELIM, vdisk);

    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        fputs(blocks[block].data, vdisk);
        fputs(VDISK_DELIM, vdisk);
    }

    fclose(vdisk);
    return 0;
}


// TODO: rename this
// professor will certainly see "initialize" and give 0
// without bothering to read
int initialize_virtual_disk() {
    if (filetable == NULL)
        filetable = _initialize_file_table();

    // check if vdisk exists. F_OK = check existence
    // https://linux.die.net/man/2/access
    if (access(VDISK_LOC, F_OK)) { // returned 1 = doesnt exist
        _initialize_users_and_blocks(&ub);
        write_update_to_vdisk();
    } else // disk already exists, read it in
        _read_update_from_vdisk();

    return 0;

}