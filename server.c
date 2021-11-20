#include "ssnfs.h"
#include "diskutil.h"
#include "serverutil.h"


// Closes a file given the file descriptor.
/*
 Assignment spec:
 "Closes the file with given file descriptor.
 After a file is closed, the user should not
 be able to read from the file or write to the file."
*/
close_output* close_file_1_svc(close_input* argp, struct svc_req* rqstp) {
	static close_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	int file_descriptor = argp->fd;

	printf("\nCLOSE: User requested file with descriptor (%d) be closed.\n", file_descriptor);

	if (!is_valid_file_descriptor(file_descriptor)) {
		sprintf(out_msg, "CLOSE: File descriptor (%d) is not open.", file_descriptor);
		printf("CLOSE: File with descriptor (%d) is already open, cannot be closed.\n", file_descriptor);
	} else {
		drop_entry_from_file_table(file_descriptor);
		sprintf(out_msg, "CLOSE: File descriptor (%d) closed.", file_descriptor);
		printf("CLOSE: File with descriptor (%d) has been closed.\n", file_descriptor);
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	printf("CLOSE: End of CLOSE.\n");

	return &result;
}


// Deletes a file given the user's name and the file name.
/*
 Assignment spec:
 "Deletes the specified file."
*/
delete_output* delete_file_1_svc(delete_input* argp, struct svc_req* rqstp) {
	static delete_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	char* user_name = argp->user_name;
	char* file_name = argp->file_name;

	printf("\nDELETE: User (%s) requested file (%s) be deleted.\n", user_name, file_name);

	// Delete from file table
	int file_index_in_filetable = get_filetable_index_of_file_name(user_name, file_name);
	if (file_index_in_filetable != -1) {
		int file_descriptor = filetable->entries[file_index_in_filetable].fileDescriptor;
		drop_entry_from_file_table(file_descriptor);
		printf("DELETE: File with name (%s) and descriptor (%d) of user (%s) dropped from file table.\n", file_name, file_descriptor, user_name);
	}

	// Now delete from vdisk

	load_or_initialize_virtual_disk();

	if (!is_valid_user_name(user_name)) {
		sprintf(out_msg, "DELETE: User (%s) unknown.", user_name);
		printf("DELETE: User (%s) unknown.\n", user_name);
	}

	else if (!is_valid_file_name(get_usersblocks_index_of_user_name(user_name), file_name)) {
		sprintf(out_msg, "DELETE: File (%s) unknown.", file_name);
		printf("DELETE: File (%s) unknown.\n", file_name);
	}

	// Username and filename both exist in usersblocks
	else {
		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
		int file_index_in_usersblocks = get_usersblocks_index_of_file(user_index_in_usersblocks, file_name);

		// Delete the file from the disk,
		// then write the blocks out to the disk with the file deleted.
		drop_file_from_vdisk(user_index_in_usersblocks, file_index_in_usersblocks);
		write_update_to_vdisk();

		sprintf(out_msg, "DELETE: File (%s) deleted.", file_name);
		printf("DELETE: File with name (%s) of user (%s) deleted from disk.\n", file_name, user_name);
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	printf("DELETE: End of DELETE.\n");

	return &result;
}


// Given a username, lists the files that user has.
/*
 Assignment spec:
 "Lists the names of all files in the user’s directory."
*/
list_output* list_files_1_svc(list_input* argp, struct svc_req* rqstp) {
	static list_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	char* user_name = argp->user_name;

	printf("\nLIST: User (%s) requested list of their files.\n", user_name);

	load_or_initialize_virtual_disk();

	if (!is_valid_user_name(user_name)) {
		sprintf(out_msg, "LIST: User (%s) unknown.", user_name);
		printf("LIST: User (%s) unknown.\n", user_name);
	}

	// User exists in usersblocks
	else {

		int user_index_in_userblocks = get_usersblocks_index_of_user_name(user_name);
		if (get_num_user_files_in_usersblocks(user_index_in_userblocks) > 0) {

			strcat(out_msg, LIST_DELIM);
			strcpy(out_msg, "LIST");

			for (int file = 0; file < MAX_USER_FILES; file++) {

				// Check if file is not just unallocated space
				if (strcmp(DEFAULT_FILE_NAME, ub.users[user_index_in_userblocks].files[file].name) != 0) {
					// TODO: Using strcat because I couldn't think of a better way. Is strcat unconventional?
					strcat(out_msg, LIST_DELIM);
					strcat(out_msg, ub.users[user_index_in_userblocks].files[file].name);
				}

			}

			printf("LIST: File names copied.\n");

		// User exists in usersblocks, but has no files.
		// TODO: should this be LIST: or ERROR: ?
		} else {
			sprintf(out_msg, "LIST: No files found in user (%s) directory.", user_name);
			printf("LIST: No files found in user (%s) directory.\n", user_name);
		}

	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	printf("LIST: End of LIST.\n");

	return &result;
}


// Marks a file as open given the user's name and file name.
// Returns the file descriptor of the opened file.
/*
 Assignment spec:
 "Opens the file with the given name in the user’s directory
 and returns a file descriptor (a positive integer) to the client.
 The file descriptor is used in subsequent reads and writes.
 If there is no file with the given file name in the user’s directory,
 it creates a file with that name and returns the file descriptor.
 If it cannot open a file for some reason
  (for example, no space on the disk to create a new file, or file table at the
  server has grown large. You can assume size of file table to be 20.)
  it returns -1 as file descriptor.
 Each user is assigned his/her own directory with his/her login name as the name of the directory.
 The login name of a user can be obtained from the password file (using the command getpwuid(getuid())->pw name).
 A newly created file is allocated 64 blocks.
 File size is fixed (i.e., files cannot grow dynamically)."
*/
// NOTE: It says I *CAN* assume file table size to be 20. I will choose not to.
// #define MAX_FT_SIZE (MAX_USER_FILES * MAX_NUM_USERS)
open_output* open_file_1_svc(open_input* argp, struct svc_req* rqstp) {
	static open_output result;
	result.fd = -1;

	char* user_name = argp->user_name;
	char* file_name = argp->file_name;

	printf("\nOPEN: User (%s) requested file (%s) be opened.\n", user_name, file_name);

	load_or_initialize_virtual_disk();

	// Does file exist already?
	int file_index_in_filetable = get_filetable_index_of_file_name(user_name, file_name);

	// File does not exist already
	if (file_index_in_filetable == -1) {

		// Does user exist already?
		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);

		// User does not exist already
		if (user_index_in_usersblocks == -1) {

			// User doesn't exist, is there room to add them?
			int num_users_in_usersblocks = get_num_users_in_usersblocks();

			// There is room to add them.
			// Add them, add their file, write these changes to the disk.
			if (num_users_in_usersblocks < MAX_NUM_USERS) {
				add_user_to_usersblocks(user_name);
				user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
				add_file_to_usersblocks(user_index_in_usersblocks, file_name);
				result.fd = add_entry_to_file_table(user_name, file_name);
				write_update_to_vdisk();
				printf("OPEN: User is unknown, created new directory for user.\n");
			}

			// There is not room to add them
			else {
				printf("OPEN: User is unknown, no room for additional users.\n");
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
				// Add the file, then write the change out to the disk.
				if (num_user_files_in_userblocks < MAX_USER_FILES) {
					add_file_to_usersblocks(user_index_in_usersblocks, file_name);
					result.fd = add_entry_to_file_table(user_name, file_name);
					write_update_to_vdisk();
					printf("OPEN: File is unknown, created new entry for file.\n");
				}

				// There is not room to add it
				else {
					printf("OPEN: File is unknown, no room for additional files.\n");
				}
			}

			// File exists in usersblocks
			else {
				result.fd = add_entry_to_file_table(user_name, file_name);
				printf("OPEN: Reopened previously-closed file.\n");
			}

		}

	// File exists already
	// Just return its file descriptor
	} else {
		result.fd = filetable->entries[file_index_in_filetable].fileDescriptor;
		printf("OPEN: Opened file with (%d) as descriptor.\n", result.fd);
	}

	result.out_msg.out_msg_len = strlen(file_name);
	result.out_msg.out_msg_val = malloc(strlen(file_name));
	strcpy(result.out_msg.out_msg_val, file_name);

	printf("OPEN: End of OPEN.\n");

	return &result;
}


// Original open_file_1_svc() given with assignment
// I didn't really understand it, so I made my own.
/*
open_output *
open_file_1_svc(open_input *argp, struct svc_req *rqstp)
{
	static open_output  result;

	result.fd=20;
	result.out_msg.out_msg_len=10;
	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
        strcpy(result.out_msg.out_msg_val, (*argp).file_name);
	printf("In server: filename recieved:%s\n",argp->file_name);
	printf("In server username received:%s\n",argp->user_name);
	//	fflush((FILE *) 1);
	return &result;
}
}
*/


// Reads a file given its file descriptor, the users name,
// and the number of bytes that should be read.
// Will also move a file's pointer position accordingly.
/*
 Assignment spec:
 "Reads the specified number of bytes from the current position
 and returns it to the client and advances the file pointer by
 the number of bytes read.
 Uses variable length buffer.
 Returns appropriate error message if trying to read past the
 end of file, file descriptor passed was not correct, etc.."
*/
read_output* read_file_1_svc(read_input* argp, struct svc_req* rqstp) {
	static read_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	// When a read is called without closing/reopening the file,
	// the file pointer will not have moved, so unwritten bytes in the file will be read.
	// The spec states the file should always be 64 bytes, so these space characters are a valid part of the file.
	// A warning will be shown when the read grabbed only spaces.
	char read_chars_all_spaces = true;

	int bytes_to_read = argp->numbytes;
	char out_data[bytes_to_read+1];
	int last_buffer_index = 0;
	bool read = false;

	int file_descriptor = argp->fd;
	char* user_name = argp->user_name;

	printf("\nREAD: User (%s) requested file with descriptor (%d) have bytes (%d) read from it.\n", user_name, file_descriptor, bytes_to_read);

	load_or_initialize_virtual_disk();

	// Does this file exist?
	if (!is_valid_file_descriptor(file_descriptor)) {
		sprintf(out_msg, "READ: File descriptor (%d) unknown.", file_descriptor);
		printf("READ: File descriptor (%d) unknown.\n", file_descriptor);

	// File exists. Does the user exist?
	} else if (!is_valid_user_name(user_name)) {
		sprintf(out_msg, "READ: User (%s) unknown.", user_name);
		printf("READ: User (%s) unknown.\n", user_name);

	// File and user both exist. Read.
	} else {

		int file_index_in_filetable = get_filetable_index_of_file_descriptor(file_descriptor);
		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
		char* file_name = filetable->entries[file_index_in_filetable].fileName;
		int file_index_in_usersblocks = get_usersblocks_index_of_file(user_index_in_usersblocks, file_name);

		// Weird bug I got a couple times.
		// I guess file descriptor can exist but file can't?
		if (!is_valid_file_name(user_index_in_usersblocks, file_name)) {
			sprintf(out_msg, "READ: File (%s) unknown.", file_name);
			printf("READ: File (%s) unknown.\n", file_name);

		} else {

			int old_pos = filetable->entries[file_index_in_filetable].filePointerPos;
			int new_pos = old_pos + bytes_to_read;

			printf("READ: File pointer will be moved from (%d) to (%d) totaling (%d) bytes.\n", old_pos, new_pos, bytes_to_read);

			if (new_pos > MAX_POINTER_POS) {
				sprintf(out_msg, "READ: Cannot read from (%s) past EOF.", file_name);
				printf("READ: Cannot read from (%s) past EOF.\n", file_name);

			} else {

				int block_index_in_usersblocks = old_pos / BLOCK_SIZE;  // block 1, 2, 3... within file
				int pos_in_block_in_usersblocks = old_pos % BLOCK_SIZE; // byte 1, 2, 3... within block 1, 2, 3...

				// If the position in the block is at the end of the block,
				// move on to the next block.
				int pos_of_block_in_blocks;
				if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
					block_index_in_usersblocks += 1;
					pos_in_block_in_usersblocks = 0;
				}

				printf("READ: Block (%d) at position (%d) is start of read.\n", block_index_in_usersblocks, pos_in_block_in_usersblocks);

				printf("READ: Read bytes ");

				int byte_index_in_buffer = 0;
				while (byte_index_in_buffer < bytes_to_read) {

					if (block_index_in_usersblocks >= FILE_SIZE) {
						break;
					}

					// read the next byte of the file
					else {
						pos_of_block_in_blocks = ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block_index_in_usersblocks];
						out_data[byte_index_in_buffer] = blocks[pos_of_block_in_blocks].data[pos_in_block_in_usersblocks];
						if (out_data[byte_index_in_buffer] != ' ') {
							read_chars_all_spaces = false;
						}
						pos_in_block_in_usersblocks += 1;
						read = true;
						printf("%d ", byte_index_in_buffer);
					}

					// move on to next block if needed
					if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
						block_index_in_usersblocks += 1;
						pos_in_block_in_usersblocks = 0;
						printf("READ: Moved to position (%d) in next block in file (%s).\n", pos_in_block_in_usersblocks, file_name);
					}

					byte_index_in_buffer += 1;
					last_buffer_index = byte_index_in_buffer;
				}

				printf("from buffer.\n");
				sprintf(out_msg, "READ: Read successful.");
				write_update_to_file_pointer_pos(user_name, file_name, file_descriptor, pos_in_block_in_usersblocks);

			}
		}
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	if (read) {
		out_data[last_buffer_index] = '\0'; // null terminate string
		result.buffer.buffer_len = strlen(out_data);
		result.buffer.buffer_val = malloc(strlen(out_data));
		strncpy(result.buffer.buffer_val, out_data, bytes_to_read+1); // +1 for null terminate
		printf("READ: Read contents (%s) of length (%ld) from file.\n", result.buffer.buffer_val, strlen(result.buffer.buffer_val));
	}

	if (read_chars_all_spaces) {
		print_read_pointer_pos_warning();
	}

	printf("READ: End of READ.\n");

	return &result;
}


// Given a username, a file descriptor, and a number of bytes to write,
// writes the given bytes to the file and adjusts the file pointer accordingly.
/*
 Assignment spec:
 "writes the specified number of bytes from the buffer
 to the file represented by the file descriptor
 from the current position and
 advances the file pointer the number of bytes written.
 Uses variable length buffer.
 Returns appropriate error message if write fails."
*/
write_output* write_file_1_svc(write_input* argp, struct svc_req* rqstp) {
	static write_output result;
	char out_msg[OUT_MSG_BUF_LEN];

	printf("\nWRITE: User (%s) requested file with descriptor (%d) have contents (%s) to length (%d) written.\n", argp->user_name, argp->fd, argp->buffer.buffer_val, argp->numbytes);

	load_or_initialize_virtual_disk();

	if (!is_valid_file_descriptor(argp->fd)) {
		sprintf(out_msg, "WRITE: File descriptor (%d) unknown.", argp->fd);
		printf("WRITE: File descriptor (%d) unknown.", argp->fd);

	} else if (!is_valid_user_name(argp->user_name)) {
		sprintf(out_msg, "WRITE: User (%s) unknown.", argp->user_name);
		printf("WRITE: User (%s) unknown.", argp->user_name);

	// TODO: This condition is absurdly long. It does work though...
	} else if (!is_valid_file_name(get_usersblocks_index_of_user_name(argp->user_name), filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName)) {
		sprintf(out_msg, "WRITE: File (%s) unknown.", filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName);
		printf("WRITE: File (%s) unknown.", filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName);


	// User, file, and file descriptor are all valid
	} else {

		char* user_name = argp->user_name;
		int file_descriptor = argp->fd;
		int bytes_to_write = argp->numbytes;

		int user_index_in_usersblocks = get_usersblocks_index_of_user_name(user_name);
		int file_index_in_filetable = get_filetable_index_of_file_descriptor(file_descriptor);
		char* file_name = filetable->entries[file_index_in_filetable].fileName;
		int file_index_in_usersblocks = get_usersblocks_index_of_file(user_index_in_usersblocks, file_name);

		int old_pos = filetable->entries[file_index_in_filetable].filePointerPos;
		int new_pos = old_pos - 1 + bytes_to_write;

		printf("WRITE: File pointer will be moved from (%d) to (%d) totaling (%d) bytes.\n", old_pos, new_pos, bytes_to_write);

		if (new_pos > MAX_POINTER_POS) {
			sprintf(out_msg, "WRITE: Cannot write to (%s) past EOF.", file_name);

		} else {

			int block_index_in_usersblocks = old_pos / BLOCK_SIZE;  // block 1, 2, 3... within file
			int pos_in_block_in_usersblocks = old_pos % BLOCK_SIZE; // byte 1, 2, 3... within block 1, 2, 3...

			// If the position in the block is at the end of the block,
			// move on to the next block.
			if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
				block_index_in_usersblocks += 1;
				pos_in_block_in_usersblocks = 0;
			}

			int byte_index_in_buffer = 0;
			while (byte_index_in_buffer < bytes_to_write) {

				if (block_index_in_usersblocks >= FILE_SIZE) {
					sprintf(out_msg, "WRITE: Reached EOF while writing to (%s).", file_name);
					printf("WRITE: Reached EOF while writing to (%s).", file_name);
					break;
				}

				else {
					// TODO: also works, but is ridiculously long
					blocks[ub.users[user_index_in_usersblocks].files[file_index_in_usersblocks].blocks[block_index_in_usersblocks]].data[pos_in_block_in_usersblocks] =
						argp->buffer.buffer_val[byte_index_in_buffer];
					pos_in_block_in_usersblocks += 1;
				}

				// move on to next block if needed
				if (pos_in_block_in_usersblocks == BLOCK_SIZE) {
					block_index_in_usersblocks += 1;
					pos_in_block_in_usersblocks = 0;
					printf("WRITE: Moved to position (%d) in next block in file (%s).\n", pos_in_block_in_usersblocks, file_name);
				}

				byte_index_in_buffer += 1;
			}

			write_update_to_file_pointer_pos(user_name, file_name, file_descriptor, pos_in_block_in_usersblocks);
			write_update_to_vdisk();
			sprintf(out_msg, "WRITE: Wrote (%d bytes) to (%s).", bytes_to_write, file_name);
			printf("WRITE: Wrote (%d bytes) to (%s).\n", bytes_to_write, file_name);

		}
	}

	result.out_msg.out_msg_len = strlen(out_msg);
	result.out_msg.out_msg_val = malloc(strlen(out_msg));
	strcpy(result.out_msg.out_msg_val, out_msg);

	printf("WRITE: End of WRITE.\n");

	return &result;
}
