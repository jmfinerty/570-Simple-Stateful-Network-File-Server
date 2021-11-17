#include "ssnfs.h"
#include "server.h"
#include "diskutil.h"
#include "serverutil.h"


open_output*
open_file_1_svc(open_input* argp, struct svc_req* rqstp) {
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


read_output*
read_file_1_svc(read_input* argp, struct svc_req* rqstp) {
	static read_output result;



	return &result;
}


write_output* write_file_1_svc(write_input* argp, struct svc_req* rqstp) {
	static write_output result;
	char out_message[1024];

	initialize_virtual_disk();

	if (!is_valid_file_descriptor(argp->fd))
		sprintf(out_message, "ERROR: File descriptor (%d) unknown.", argp->fd);
	else if (!is_valid_user_name(argp->user_name))
		sprintf(out_message, "ERROR: User (%s) unknown.", argp->user_name);
	else if (!is_valid_file_name(get_usersblocks_index_of_user_name(argp->user_name), filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName))
		sprintf(out_message, "ERROR: File (%s) unknown.", filetable->entries[get_filetable_index_of_file_descriptor(argp->fd)].fileName);

	else {
		// no errors yet, try to write
	}

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
