#include "ssnfs.h"
#include "server.h"
#include "diskutil.h"


int _check_file_descriptor(int file_descriptor) {

	// file_descriptor should be non-negative
	// and should be within the range of possible entries in FileTable
	if (file_descriptor < 0 || file_descriptor > MAX_FT_SIZE) {
		return 1;
	} else { // file descriptor should exist in FileTable
		for (int file = 0; file < MAX_FT_SIZE; file++) {
			if (filetable->entries[file].fileDescriptor == file_descriptor) {
				return 0;
			}
		}
	}

	return 1;
}


open_output*
open_file_1_svc(open_input* argp, struct svc_req* rqstp) {
	static open_output result;

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


read_output*
read_file_1_svc(read_input* argp, struct svc_req* rqstp) {
	static read_output result;



	return &result;
}


write_output* write_file_1_svc(write_input* argp, struct svc_req* rqstp) {
	static write_output result;

	int file_descriptor = argp->fd;
	_check_file_descriptor(file_descriptor);

	initialize_virtual_disk();


	return &result;
}


list_output*
list_files_1_svc(list_input* argp, struct svc_req* rqstp) {
	static list_output result;



	return &result;
}


delete_output*
delete_file_1_svc(delete_input* argp, struct svc_req* rqstp) {
	static delete_output result;



	return &result;
}


close_output*
close_file_1_svc(close_input* argp, struct svc_req* rqstp) {
	static close_output result;



	return &result;
}
