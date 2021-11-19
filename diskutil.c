/*

This file stores helper functions used by the server.
These helper functions deal with disk and memory management.

They are primarily setter functions.
For getter functions, see serverutil.c.

- Setting values from the filetable and from the blocks in memory
- Writing updates to the filetable and to the virtual disk file
- Initializing in-memory values like the filetable and blocks
- Initializing the virtual disk file

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "diskutil.h"


Block blocks[MAX_NUM_BLOCKS];   // stores actual blocks and their data
FileTable* filetable = NULL;    // stores information about users and files they own
UsersBlocks ub;                 // stores information about users and block allocation


// Read information in from existing virtual disk file.
// This will populate usersblocks and blocks.
// Should only be called after checking that the disk file actually exists.
int _read_update_from_vdisk() {
    printf("    Reading update from virtual disk... "); fflush(stdout);

    FILE* vdisk = fopen(VDISK_LOC, "r");

    // HACK
    // Was having an issue where read in usernames, filenames would always end in VDISK_DELIM.
    // This meant any strcmp() check with the calling user's name or filename would fail,
    // since the provided username/filename wouldn't have had VDISK_DELIM at the end.
    // This project is due in 24 hours. I'm sure there's a fix for this, but I'm not going to fix it now.
    char delim_buf[DELIM_BUF_SIZE];

    // Read in first row of vdisk, which contains information about block allocation.
    // +1 so fgets will read all the way to delimiter and stop reading.
    fgets(ub.blocks, MAX_NUM_BLOCKS+1, vdisk);

    // HACK: Read in VDISK_DELIM character
    fgets(delim_buf, DELIM_BUF_SIZE, vdisk);

    // There should be MAX_NUM_USERS and MAX_NUM_FILES in the vdisk
    for (int user = 0; user < MAX_NUM_USERS; user++) {

        // Read username line from file
        fgets(ub.users[user].name, MAX_USER_NAME_LEN, vdisk);

        // Strip newline from end of read-in username
        // source: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        ub.users[user].name[strcspn(ub.users[user].name, "\n")] = 0;

        for (int file = 0; file < MAX_USER_FILES; file++) {

            // Read this user's filename at index <file> from file
            fgets(ub.users[user].files[file].name, MAX_FILE_NAME_LEN, vdisk);

            // Strip newline from end of read-in filename
            // source: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
            ub.users[user].files[file].name[strcspn(ub.users[user].files[file].name, "\n")] = 0;

            // Read in the block information after the file name line
            for (int block = 0; block < FILE_SIZE; block++) {
                fscanf(vdisk, "%d ", &ub.users[user].files[file].blocks[block]);
            }
        }
    }

    // HACK
    // When this is anything but 1, the first character of the first file will always get cut off
    fgets(delim_buf, 1, vdisk);
    //fgets(delim_buf, 1 DELIM_BUF_SIZE, vdisk);

    // Read in the block information
    // starts at end of users/file information
    // ends at EOF
    for (int b = 0; b < MAX_NUM_BLOCKS; b++) {
        fgets(blocks[b].data, BLOCK_SIZE+1, vdisk);

        // HACK: Read in VDISK_DELIM character at end of users/file information
        fgets(delim_buf, DELIM_BUF_SIZE, vdisk);
    }

    printf("DONE.\n");

    fclose(vdisk);
    return 0;
}


// Used when a new user asked to create a new file.
// Given a file name and a user,
// iterates through the file table until an unallocated user slot is found,
// then finds an empty file slot in that user's files and allocates both.
// If there are no empty user slots or file slots in empty users, return -1.
// Otherwise return the file descriptor of the created file.
int file_descriptors_pos = INIT_FILE_DESCRIP; // tracks the current file descriptor, so new ones will always be unique
int add_entry_to_file_table(char* user_name, char* file_name) {
    for (int entry = 0; entry < MAX_FT_SIZE; entry++)
        if (strcmp(filetable->entries[entry].ownerUserName, DEFAULT_USER_NAME) == 0)
            if (strcmp(filetable->entries[entry].fileName, DEFAULT_FILE_NAME) == 0) {
                file_descriptors_pos += 1;
                filetable->entries[entry].fileDescriptor = file_descriptors_pos;
                filetable->entries[entry].filePointerPos = 0;
                strcpy(filetable->entries[entry].fileName, file_name);
                strcpy(filetable->entries[entry].ownerUserName, user_name);
                printf("    Added entry to file table for user (%s) file (%s).\n", user_name, file_name);
                return file_descriptors_pos;
            }
    printf("    Could not find slot in file table for user (%s) file (%s).\n", user_name, file_name);
    return -1;
}


// Given an index of a user in usersblocks, and a name of a file to create for them,
// find the first empty file slot and create the file there.
// If there are no empty slots, return -1. Otherwise, return the index the file was created at.
// Additionally, allocate space in blocks for this file.
// The vdisk should have this change written out to it after this function is called,
// so call write_update_to_vdisk() after this.
// I did not code that into this function in case multiple changes need to be made before the write.
int add_file_to_usersblocks(int user_index_in_usersblocks, char* file_name) {

    // Find a file with the default name -- i.e., an unallocated space
    for (int file = 0; file < MAX_USER_FILES; file++) {
        if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_usersblocks].files[file].name) == 0) {

            // Write out the file name
            strcpy(ub.users[user_index_in_usersblocks].files[file].name, file_name);

            // Iterate through usersblocks until an empty block to write to is found
            int empty_block_index = 0;
            while (ub.users[user_index_in_usersblocks].files[file].blocks[empty_block_index] != 0)
                empty_block_index += 1;

            // Assign bytes in the empty block to this.
            // Stop when there aren't any more blocks left, or the whole file is allocated
            int assigned = 0;
            for (int block = 0; block < MAX_NUM_BLOCKS; block++) {

                // max file size reached, whole file's space is allocated, stop
                if (assigned >= FILE_SIZE)
                    break;

                // if this block is unallocated, mark it as allocated
                if (ub.blocks[block] == '0') {
                    ub.users[user_index_in_usersblocks].files[file].blocks[empty_block_index] = block; // allocate
                    empty_block_index += 1;
                    ub.blocks[block] = '1'; // mark as allocated
                    assigned += 1;
                }
            }
            printf("    Added file (%s) at index (%d).\n", file_name, file);
            return file;
        }
    }
    printf("    Could not find slot to add file (%s).\n", file_name);
    return -1;
}


// Given a username,
// create an entry for that user in usersblocks at the first empty slot.
// If there are no empty slots, return -1. Otherwise, return index user was created at.
int add_user_to_usersblocks(char* user_name) {
    for (int user = 0; user < MAX_NUM_USERS; user++)
        if (strcmp(DEFAULT_USER_NAME, ub.users[user].name) == 0) {
            strcpy(ub.users[user].name, user_name);
            printf("    Added user (%s) at index (%d).\n", user_name, user);
            return user;
        }
    printf("    Could not find slot to add user (%s).\n", user_name);
    return -1;
}


// Given a unique file descriptor for some file in the filetable,
// iterate through the filetable until a a file with that descriptor is found,
// then reset its information to default values.
int drop_entry_from_file_table(int file_descriptor) {
    for (int entry = 0; entry < MAX_FT_SIZE; entry++) {
        if (filetable->entries[entry].fileDescriptor == file_descriptor) {
            //file_descriptors_pos -= 1; // don't decrement actually, want file descriptors to be distinct
            printf("    Removed file (%s) with descriptor (%d) from file table.\n", filetable->entries[entry].fileName, filetable->entries[entry].fileDescriptor);
            filetable->entries[entry].fileDescriptor = 0;
            filetable->entries[entry].filePointerPos = 0;
            strcpy(filetable->entries[entry].fileName, DEFAULT_FILE_NAME);
            strcpy(filetable->entries[entry].ownerUserName, DEFAULT_USER_NAME);
            return 0;
        }
    }
    printf("    Could not remove file with descriptor (%d) from file table.\n", file_descriptor);
    return 1;
}


// Given the index of a user in usersblocks,
// and the index of file in that user's files in usersblocks,
// delete that file and clear the space on the disk that was allocated to it.
// Also delete the file's entry in usersblocks.
int drop_file_from_vdisk(int user_index_in_usersblocks, int file_index_in_usersblocks) {
    strcpy(ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].name, DEFAULT_FILE_NAME);
    for (int block = 0; block < FILE_SIZE; block++)
        if (ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block] != 0) { // block is allocated
            ub.blocks[ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block]] = '0'; // set it to unallocated
            memset(blocks[ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block]].data, ' ', BLOCK_SIZE);
            ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block] = 0;
            printf("    Removed file at index (%d) from user at index (%d) on disk.\n", user_index_in_usersblocks, file_index_in_usersblocks);
        }
    return 0;
}


// Given a username and a file descriptor,
// finds that file descriptor in the user's files
// and writes the given file pointer position to the file it found
int write_update_to_file_pointer_pos(char* user_name, char* file_name, int file_descriptor, int file_pointer_pos) {
    for (int entry; entry < MAX_FT_SIZE; entry++)
        if (strcmp(filetable->entries[entry].ownerUserName, user_name) == 0)
            if (filetable->entries[entry].fileDescriptor == file_descriptor) {
                printf("    Moved file pointer of file with name (%s) and descriptor (%d) of user (%s) from position (%d) to (%d).", file_name, file_descriptor, user_name, filetable->entries[entry].filePointerPos, file_pointer_pos);
                filetable->entries[entry].filePointerPos = file_pointer_pos;
            }
    return 0;
}


// Writes the current state of usersblocks to the virtual disk file.
// The file is structured like so, with each of these lines separated by VDISK_DELIM:
// ub.blocks:   0111111..0000000000...
// username:    username
// filename:    filename
// file blocks: 1 2 3 4 5 6 7 8 9... 64
// [repeat username - file blocks]
// [blocks bytes]
// I feel like this is an intuitive format. It is basically Index->Directories->Files->FileContentInfo, then actual info at bottom.
// Sort of like file details and then actual file.
int write_update_to_vdisk() {

    printf("    Writing update to disk... "); fflush(stdout);

    FILE* vdisk = fopen(VDISK_LOC, "w");

    fputs(ub.blocks, vdisk);
    fputs(VDISK_DELIM, vdisk);

    // Write out every user and their name,
    // every file and its name,
    // then every block index for that file.
    for (int user = 0; user < MAX_NUM_USERS; user++) {
        fputs(ub.users[user].name, vdisk);
        fputs(VDISK_DELIM, vdisk);

        for (int file = 0; file < MAX_USER_FILES; file++) {
            fputs(ub.users[user].files[file].name, vdisk);
            fputs(VDISK_DELIM, vdisk);

            for (int block = 0; block < FILE_SIZE; block++) {
                fprintf(vdisk, "%d ", ub.users[user].files[file].blocks[block]);
                //fputs(ub.users[user].files[file].blocks[block], vdisk);
            }

            fputs(VDISK_DELIM, vdisk);
        }
    }

    fputs(VDISK_DELIM, vdisk);

    // Write out blocks data
    for (int block = 0; block < MAX_NUM_BLOCKS; block++) {
        fputs(blocks[block].data, vdisk);
        fputs(VDISK_DELIM, vdisk);
    }

    printf("DONE.\n");

    fclose(vdisk);
    return 0;
}


// Check if the virtual disk exists
// If it does, load it in. If it doesn't, create it.
int load_or_initialize_virtual_disk() {

    // Check if the filetable has not been initialized yet.
    // If it has not been, allocate space for it, then
    // assign default values to each entry in it.
    if (filetable == NULL) {
        printf("    File table does not exist, initializing... "); fflush(stdout);
        filetable = (FileTable*) malloc(sizeof(FileTable)+1);
        for (int i = 0; i < MAX_FT_SIZE; i++) {
            strcpy(filetable->entries[i].ownerUserName, DEFAULT_USER_NAME);
            strcpy(filetable->entries[i].fileName, DEFAULT_FILE_NAME);
            filetable->entries[i].fileDescriptor = 0;
            filetable->entries[i].filePointerPos = 0;
        }
        printf("DONE.\n");
    }

    // Check if the virtual disk file exists.
    // (F_OK = check existence https://linux.die.net/man/2/access)
    if (access(VDISK_LOC, F_OK)) {

        printf("    Virtual disk at location (%s) does not exist, initializing... ", VDISK_LOC); fflush(stdout);

        // Access returned 1, so disk file does not exist

        // Copy '0' into index of every block to indicate
        // that the block is not allocated.
        memset(ub.blocks, '0', MAX_NUM_BLOCKS);

        // Copy spaces (indicating no value stored there) into every block
        for (int b = 0; b < MAX_NUM_BLOCKS; b++) {
            memset(blocks[b].data, ' ', BLOCK_SIZE);
        }

        // Iterate through every user.
        // Give them the default username, then go through each of their sile slots.
        // Give each of those files the default filename, then
        // mark each of that file's blocks unallocated.
        for (int user = 0; user < MAX_NUM_USERS; user++) {
            strcpy(ub.users[user].name, DEFAULT_USER_NAME);

            for (int file = 0; file < MAX_USER_FILES; file++) {
                strcpy(ub.users[user].files[file].name, DEFAULT_FILE_NAME);

                for (int block = 0; block < FILE_SIZE; block++) {
                    ub.users[user].files[file].blocks[block] = 0;
                }
            }
        }

        printf("DONE.\n");

        write_update_to_vdisk();

    } else {

        // Disk file does exist, so read it in
        _read_update_from_vdisk();

    }

    return 0;

}