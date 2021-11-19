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
int get_num_user_files_in_usersblocks(int user_index_in_usersblocks) {
    int result = 0;
    for (int file = 0; file < MAX_USER_FILES; file++)
        if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_usersblocks].files[file].name) != 0)
            result += 1;
    return result;
}


// Returns how many users currently exist in usersblocks.
// TODO: this isn't needed. Could keep global tracker num_users, ++ when user added, -- when dropped.
int get_num_users_in_usersblocks() {
    int result = 0;
    for (int user = 0; user < MAX_NUM_USERS; user++)
        if (strcmp(DEFAULT_USER_NAME, ub.users[user].name) != 0)
            result += 1;
    return result;
}


// Given a user's index in the usersblocks and a name of one of their files,
// returns -1 if the filename doesn't exist for this user,
// otherwise returns the index of the file among the user's files.
int get_usersblocks_index_of_file(int user_index_in_usersblocks, char* file_name) {
    for (int file = 0; file < MAX_USER_FILES; file++) {
        if (strcmp(file_name, ub.users[user_index_in_usersblocks].files[file].name) == 0)
            return file;
    }
    return -1;
}


// Given a username,
// returns -1 if that user doesn't exist,
// otherwise returns index of that user in the usersblocks.
int get_usersblocks_index_of_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, ub.users[user].name) == 0) // 0 = exact match
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
		if (strcmp(file_name, ub.users[user_index].files[file].name) == 0)
			return true;
	return false;
}


// Checks if a user name is valid.
// Returns false if the username doesn't exist in the usersblocks.
// Otherwise returns true.
bool is_valid_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, ub.users[user].name) == 0) // 0 = exact match
			return true;
	return false;
}
