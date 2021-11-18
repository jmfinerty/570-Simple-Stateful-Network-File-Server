#include <string.h>
#include "diskutil.h"
#include "serverutil.h"


int get_filetable_index_of_file_descriptor(int file_descriptor) {
	for (int file = 0; file < MAX_FT_SIZE; file++)
		if (filetable->entries[file].fileDescriptor == file_descriptor)
			return file;
	return -1;
}


int get_filetable_index_of_user_file(char* user_name, char* file_name) {
    for (int entry = 0; entry < MAX_FT_SIZE; entry++)
        if (strcmp(user_name, filetable->entries[entry].ownerUserName) == 0)
            if (strcmp(file_name, filetable->entries[entry].fileName) == 0)
                return entry;
    return -1;
}


int get_usersblocks_index_of_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, ub.users[user].name) == 0) // 0 = exact match
			return user;
	return -1;
}


int get_usersblocks_index_of_file(int user_index_in_usersblocks, char* file_name) {
    for (int file = 0; file < MAX_USER_FILES; file++) {
        if (strcmp(file_name, ub.users[user_index_in_usersblocks].files[file].name) == 0)
            return file;
    }
    return -1;
}


int get_num_user_files_in_usersblocks(int user_index_in_usersblocks) {
    int result = 0;
    for (int file = 0; file < MAX_USER_FILES; file++)
        if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_usersblocks].files[file].name) != 0)
            result += 1;
    return result;
}


int get_num_users_in_usersblocks() {
    int result = 0;
    for (int user = 0; user < MAX_NUM_USERS; user++)
        if (strcmp(DEFAULT_USER_NAME, ub.users[user].name) != 0)
            result += 1;
    return result;
}


bool is_valid_file_descriptor(int file_descriptor) {
	// file_descriptor should be non-negative
	// and should be within the range of possible entries in FileTable
	if (file_descriptor < 0 || file_descriptor > MAX_FT_SIZE)
		return false;
	else // file descriptor should exist in FileTable
		for (int file = 0; file < MAX_FT_SIZE; file++)
			if (filetable->entries[file].fileDescriptor == file_descriptor)
				return true;
	return false;
}


bool is_valid_user_name(char* user_name) {
	for (int user = 0; user < MAX_NUM_USERS; user++)
		if (strcmp(user_name, ub.users[user].name) == 0) // 0 = exact match
			return true;
	return false;
}


bool is_valid_file_name(int user_index, char* file_name) {
	for (int file = 0; file < MAX_USER_FILES; file++)
		if (strcmp(file_name, ub.users[user_index].files[file].name) == 0) // 0 = exact match
			return true;
	return false;
}