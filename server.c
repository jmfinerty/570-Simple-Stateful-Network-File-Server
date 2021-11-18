#include "ssnfs.h"
#include "diskutil.h"
#include "serverutil.h"


open_output* open_file_1_svc(open_input* argp, struct svc_req* rqstp) {
	static open_output result;

	initialize_virtual_disk();

	char* user_name = argp->user_name;
	char* file_name = argp->file_name;

	result.fd = -1;

	// Does file exist already?
	int file_index_in_filetable = get_filetable_index_of_user_file(user_name, file_name);

	// File does not exist already
	if (file_index_in_filetable == -1) {

		// Does user exist already?
		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);

		// User does not exist already
		if (user_index_in_usersblocks == -1) {

			// User doesn't exist, is there room to add them?
			int num_users_in_usersblocks = get_num_users_in_usersblocks();

			// There is room to add them.
			if (num_users_in_usersblocks < MAX_NUM_USERS) {
				add_user_to_usersblocks(user_name);
				user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
				add_file_to_usersblocks(user_index_in_usersblocks, file_name);
				write_update_to_vdisk();
				result.fd = add_entry_to_file_table(user_name, file_name);
			}

			// There is not room to add them
			else {
				// cant do anything, return -1
			}

		// User exists already
		} else {

			// Does the file exist in usersblocks?
			int file_index_in_usersblocks = get_usersblocks_index_of_file(user_index_in_usersblocks, file_name);

			// File does not exist in usersblocks
			if (file_index_in_usersblocks == -1) {

				// Is there room to add it?
				int num_user_files_in_userblocks = get_num_user_files_in_usersblocks(user_index_in_usersblocks);

				// There is room to add it
				if (num_user_files_in_userblocks < MAX_USER_FILES) {
					add_file_to_usersblocks(user_index_in_usersblocks, file_name);
					write_update_to_vdisk();
					result.fd = add_entry_to_file_table(user_name, file_name);
				}

				// There is not room to add it
				else {
					// cant do anything, return -1
				}
			}

			// File exists in usersblocks
			else {
				result.fd = add_entry_to_file_table(user_name, file_name);
			}

		}

	// File exists already
	} else {
		result.fd = filetable->entries[file_index_in_filetable].fileDescriptor;
	}

	return &result;
}

/*
open_output* open_file_1_svc(open_input* argp, struct svc_req* rqstp) {
	static open_output result;
	initialize_virtual_disk();

	result.fd = 20;
	result.out_msg.out_msg_len = 10;

	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_val=(char*) malloc(result.out_msg.out_msg_len);
	strcpy(result.out_msg.out_msg_val, (*argp).file_name);

	printf("In server: filename recieved:%s\n", argp->file_name);
	printf("In server username received:%s\n", argp->user_name);

	//	fflush((FILE *) 1);
	return &result;
}
*/


read_output*
read_file_1_svc(read_input* argp, struct svc_req* rqstp) {
	static read_output result;



	return &result;
}


write_output* write_file_1_svc(write_input* argp, struct svc_req* rqstp) {
	static write_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	initialize_virtual_disk();

	if (!is_valid_file_descriptor(argp->fd))
		sprintf(out_msg, "ERROR: File descriptor (%d) unknown.", argp->fd);
	else if (!is_valid_user_name(argp->user_name))
		sprintf(out_msg, "ERROR: User (%s) unknown.", argp->user_name);
	else if (!is_valid_file_name(get_usersblocks_index_of_user_name(argp->user_name), filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName))
		sprintf(out_msg, "ERROR: File (%s) unknown.", filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName);

	else {
		int file_descriptor = argp->fd;
		char* user_name = argp->user_name;
		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
		int file_index_in_filetable = get_filetable_index_of_file_descriptor(file_descriptor);
		char* file_name = filetable->entries[file_index_in_filetable].fileName;
		int file_index_in_usersblocks = get_usersblocks_index_of_file(user_index_in_usersblocks, file_name);
		int bytes_to_write = argp->numbytes;
		int old_pos = filetable->entries[file_index_in_usersblocks].filePointerPos;
		int new_pos = old_pos - 1 + bytes_to_write;

		if (new_pos > MAX_POINTER_POS)
			sprintf(out_msg, "ERROR: Cannot write to (%s) past EOF.", file_name);
		else {
			int block_index_in_usersblocks = old_pos / BLOCK_SIZE;  // block 1, 2, 3... within file
			int pos_in_block_in_usersblocks = old_pos % BLOCK_SIZE; // byte 1, 2, 3... within block 1, 2, 3...
			if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
				block_index_in_usersblocks += 1;
				pos_in_block_in_usersblocks = 0;
			}

			int byte_index_in_buffer = 0;
			while (byte_index_in_buffer < bytes_to_write) {
				if (block_index_in_usersblocks > FILE_SIZE) {
					sprintf(out_msg, "ERROR: Reached EOF while writing to (%s).", file_name);
					break;
				}
				else {
					blocks[ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block_index_in_usersblocks]].data[pos_in_block_in_usersblocks] =
						argp->buffer.buffer_val[byte_index_in_buffer];
					pos_in_block_in_usersblocks += 1;
				}
				if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
					block_index_in_usersblocks += 1;
					pos_in_block_in_usersblocks = 0;
				}
				byte_index_in_buffer += 1;
			}
			write_update_to_vdisk();
			write_update_to_filetable(user_name, file_name, file_descriptor, pos_in_block_in_usersblocks);
			sprintf(out_msg, "WRITE: Wrote (%d bytes) to (%s).", bytes_to_write, file_name);
		}
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	return &result;
}


list_output*
list_files_1_svc(list_input* argp, struct svc_req* rqstp) {
	static list_output result;
	char out_msg[OUT_MSG_BUF_LEN];
	char* user_name = argp->user_name;

	initialize_virtual_disk();

	if (!is_valid_user_name(user_name))
		sprintf(out_msg, "ERROR: User (%s) unknown.", user_name);
	else {
		int user_index_in_userblocks = get_usersblocks_index_of_user_name(user_name);
		if (get_num_user_files_in_usersblocks(user_index_in_userblocks) > 0) {
			strcpy(out_msg, "LIST");
			strcat(out_msg, LIST_DELIM);
			for (int file = 0; file < MAX_USER_FILES; file++)
				if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_userblocks].files[file].name) != 0) {
					strcat(out_msg, ub.users[user_index_in_userblocks].files[file].name);
					strcat(out_msg, LIST_DELIM);
				}
		} else
			sprintf(out_msg, "ERROR: No files found in user (%s) directory.", user_name);
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	return &result;
}


delete_output* delete_file_1_svc(delete_input* argp, struct svc_req* rqstp) {
	static delete_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	char* user_name = argp->user_name;
	char* file_name = argp->file_name;

	// Delete from file table
	int file_index_in_filetable = get_filetable_index_of_file_name(user_name, file_name);
	if (file_index_in_filetable != -1)
		drop_entry_from_file_table(filetable->entries[file_index_in_filetable].fileDescriptor);
	else
		sprintf(out_msg, "ERROR: File (%s) does not exist.", file_name);

	return &result;
}


close_output* close_file_1_svc(close_input* argp, struct svc_req* rqstp) {
	static close_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	int file_descriptor = argp->fd;
	if (!is_valid_file_descriptor(file_descriptor))
		sprintf(out_msg, "CLOSE: File descriptor (%d) is not open.", file_descriptor);
	else {
		drop_entry_from_file_table(file_descriptor);
		sprintf(out_msg, "CLOSE: File descriptor (%d) closed.", file_descriptor);
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	return &result;
}
