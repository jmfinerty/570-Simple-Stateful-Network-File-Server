// University of Kentucky CS570-001 "Modern Operating Systems", Fall 2021.
// Assignment 4: "Implement a Simple Stateful Network File Server"
// Due November 19th, 2021 at 11:59PM.
// Work is original and by Jacob Finerty, student ID 912307438,
// with exception of provided sample code and files.
// Any sources referenced are cited in comments.
// Last modification before submission: 19 Nov 2021, 9:27PM

/*

This file stores helper functions used by the server.
These helper functions deal with disk and memory retrieval,
primarily indices and names.

They are getter functions.
For setter functions, see diskutil.c.

- Getting indices of users or files given their names.
- Getting names of users or files given their indices.
- Checking validity of file/user names and indices.
- Getting sizes of tables.

*/

#include <stdio.h>		// Unused. only left here because I often need to add a printf() for debugging
#include <string.h>

#include "diskutil.h"
#include "serverutil.h"


// Given a file descriptor, returns the name of the associated file.
// Returns -1 if the file does not exist.
int get_filetable_index_of_file_descriptor(int file_descriptor) {
	for (int entry = 0; entry < MAX_FT_SIZE; entry++)
		if (filetable->entries[entry].fileDescriptor == file_descriptor)
			return entry;
	return -1;
}


// Given a user's name and the name of one of their files,
// Iterate through each of the user's files until the file is found.
// If file is not found, returns -1.
// Otherwise, it returns the index of the file in the user's files.
int get_filetable_index_of_file_name(char* user_name, char* file_name) {
	for (int entry = 0; entry < MAX_FT_SIZE; entry++)
        if (strcmp(user_name, filetable->entries[entry].ownerUserName) == 0)
		    if (strcmp(file_name, filetable->entries[entry].fileName) == 0)
			    return entry;
	return -1;
}


// Given the index of a user, returns how many files they have. Can return 0.
int get_num_user_files_in_di(int user_index_in_di) {
    int result = 0;
    for (int file = 0; file < MAX_USER_FILES; file++)
        if (strcmp(DEFAULT_FILE_NAME, di.users[user_index_in_di].files[file].name) != 0)
            result += 1;
    return result;
}


// Returns how many users currently exist in di.
// TODO: this isn't needed. Could keep global tracker num_users, ++ when user added, -- when dropped.
int get_num_users_in_di() {
    int result = 0;
    for (int user = 0; user < MAX_NUM_USERS; user++)
        if (strcmp(DEFAULT_USER_NAME, di.users[user].name) != 0)
            result += 1;
    return result;
}


// Given a user's index in the di and a name of one of their files,
// returns -1 if the filename doesn't exist for this user,
// otherwise returns the index of the file among the user's files.
int get_di_index_of_file(int user_index_in_di, char* file_name) {
    for (int file = 0; file < MAX_USER_FILES; file++) {
        if (strcmp(file_name, di.users[user_index_in_di].files[file].name) == 0)
            return file;
    }
    return -1;
}


// Given a username,
// returns -1 if that user doesn't exist,
// otherwise returns index of that user in the di.
int get_di_index_of_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, di.users[user].name) == 0) // 0 = exact match
			return user;
	return -1;
}


// Checks if a file descriptor is valid.
// Will return false if:
//  - the descriptor is not 0 or a positive integer
//	- the descriptor doesn't exist anywhere in the filetable
// Otherwise returns true.
bool is_valid_file_descriptor(int file_descriptor) {
	if (file_descriptor < 0) // || file_descriptor > MAX_FT_SIZE) // NOTE: deleted entries can push it above MAX_FT_SIZE
		return false;
	else // file descriptor should exist in FileTable
		for (int file = 0; file < MAX_FT_SIZE; file++)
			if (filetable->entries[file].fileDescriptor == file_descriptor)
				return true;
	return false;
}


// TODO
// Could probably replace is_valid_file_name()
// with get_filetable_index_of_file_descriptor()
// and just check based on if above functions have returned -1.


// Checks if a file name is valid.
// Returns false if this filename doesn't exist in the filetable.
// Otherwise returns true.
bool is_valid_file_name(int user_index, char* file_name) {
	for (int file = 0; file < MAX_USER_FILES; file++)
		if (strcmp(file_name, di.users[user_index].files[file].name) == 0)
			return true;
	return false;
}


// Checks if a user name is valid.
// Returns false if the username doesn't exist in the di.
// Otherwise returns true.
bool is_valid_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, di.users[user].name) == 0) // 0 = exact match
			return true;
	return false;
}


// When a read is called without closing/reopening the file,
// the file pointer will not have moved, so unwritten bytes in the file will be read.
// The spec states the file should always be 64 bytes, so these space characters are a valid part of the file.
// This will print the warning that will be shown when the read grabbed only spaces.
// I encountered this problem very often in testing,
// and it misled me into wasting time thinking my read function was broken.
// This warning reminds me (or maybe the tester) when that happens.
void print_read_pointer_pos_warning() {
		printf("\n");
		printf("!!!!!!!!!!! !!!!!!!!!!!  !!!!!!!!!!!  !!!!!!!!!!! !!!!!!!!!!!\n");
		printf("!!WARNING!! !!!!!!!!!!!  !!WARNING!!  !!!!!!!!!!! !!WARNING!!\n");
		printf("!!!!!!!!!!! !!!!!!!!!!!  !!!!!!!!!!!  !!!!!!!!!!! !!!!!!!!!!!\n");
		printf("!!!!!!!!!!!                                        !!!!!!!!!!\n");
		printf("!!!!!!!!!      Every byte read in was a space.      !!!!!!!!!\n");
		printf("!!!!!!!!!    User likely didn't close and reopen    !!!!!!!!!\n");
		printf("!!!!!!!!!    file to reset file pointer position.   !!!!!!!!!\n");
		printf("!!!!!!!!! Files are fixed at 64 blocks * 512 bytes. !!!!!!!!!\n");
		printf("!!!!!!!!!    Bytes not written to are thus empty,   !!!!!!!!!\n");
		printf("!!!!!!!!!    but are still a valid part of file     !!!!!!!!!\n");
		printf("!!!!!!!!!      and so they will still be read.      !!!!!!!!!\n");
		printf("!!!!!!!!!!                                         !!!!!!!!!!\n");
		printf("!!!!!!!!!!! !!!!!!!!!!!  !!!!!!!!!!!  !!!!!!!!!!! !!!!!!!!!!!\n");
		printf("!!WARNING!! !!!!!!!!!!!  !!WARNING!!  !!!!!!!!!!! !!WARNING!!\n");
		printf("!!!!!!!!!!! !!!!!!!!!!!  !!!!!!!!!!!  !!!!!!!!!!! !!!!!!!!!!!\n");
		printf("\n");
}