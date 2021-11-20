// University of Kentucky CS570-001 "Modern Operating Systems", Fall 2021.
// Assignment 4: "Implement a Simple Stateful Network File Server"
// Due November 22nd, 2021 at 11:59PM.
// Work is original and by Jacob Finerty, student ID 912307438,
// with exception of provided sample code and files.
// Any sources referenced are cited in comments.
// Last modification before submission: 19 Nov 2021, 9:33PM

#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "ssnfs.h"


CLIENT *clnt;

char* user_name;


void ssnfsprog_1(char* host) {
	clnt = clnt_create(host, SSNFSPROG, SSNFSVER, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror(host);
		exit (1);
	}
}


void Close(int fd) {
	close_output* result_6;
	close_input close_file_1_arg;

	close_file_1_arg.fd = fd;

	//printf("Closing file... ");
	result_6 = close_file_1(&close_file_1_arg, clnt);
	//printf("DONE.\n");

	if (result_6 == (close_output*) NULL) {
		clnt_perror(clnt, "call failed");
	}

	printf("%s\n", result_6->out_msg.out_msg_val);
}


void Delete(char* file_name) {
	delete_output* result_5;
	delete_input delete_file_1_arg;

	strcpy(delete_file_1_arg.user_name, user_name);
	strcpy(delete_file_1_arg.file_name, file_name);

	//printf("Deleting file... ");
	result_5 = delete_file_1(&delete_file_1_arg, clnt);
	//printf("DONE.\n");

	if (result_5 == (delete_output*) NULL) {
		clnt_perror (clnt, "call failed");
	}

	printf("%s\n", result_5->out_msg.out_msg_val);
}


void List() {
	list_output* result_4;
	list_input list_files_1_arg;

	strcpy(list_files_1_arg.user_name, user_name);

	//printf("Listing files... ");
	result_4 = list_files_1(&list_files_1_arg, clnt);
	//printf("DONE.\n");

	if (result_4 == (list_output*) NULL) {
		clnt_perror (clnt, "call failed");
	}

	printf("%s\n", result_4->out_msg.out_msg_val);
}


int Open(char* filename_to_open) {
	open_output* result_1;
	open_input open_file_1_arg;

	strcpy(open_file_1_arg.user_name, user_name); //getpwuid(getuid())->pw_name);
	strcpy(open_file_1_arg.file_name, filename_to_open);

	//printf("Opening file... ");
  	result_1 = open_file_1(&open_file_1_arg, clnt);
	//printf("DONE.\n");

	if (result_1 == (open_output*) NULL) {
		clnt_perror(clnt, "call failed");
	}

	printf("Opened file (%s) in directory (%s) with file descriptor (%d).\n", open_file_1_arg.user_name, result_1->out_msg.out_msg_val, result_1->fd);

	return result_1->fd;
}


void Read(int fd, char* buffer, int num_bytes_to_read) {
	read_output* result_2;
	read_input read_file_1_arg;


	strcpy(read_file_1_arg.user_name, user_name);
	read_file_1_arg.fd = fd;
	read_file_1_arg.numbytes = num_bytes_to_read;

	//printf("Reading file... ");
	result_2 = read_file_1(&read_file_1_arg, clnt);
	//printf("DONE.\n");

	if (result_2 == (read_output*) NULL) {
		clnt_perror(clnt, "call failed");
	}

	printf("%s\n", result_2->out_msg.out_msg_val);

	strcat(buffer, result_2->buffer.buffer_val);

}


void Write(int fd, char* buffer, int num_bytes_to_write) {
	write_output* result_3;
	write_input write_file_1_arg;

	strcpy(write_file_1_arg.user_name, user_name);
	write_file_1_arg.fd = fd;

	write_file_1_arg.numbytes = num_bytes_to_write;
	if (num_bytes_to_write > strlen(buffer))
		write_file_1_arg.numbytes = strlen(buffer);

	write_file_1_arg.buffer.buffer_len = strlen(buffer);
	write_file_1_arg.buffer.buffer_val = malloc(strlen(buffer));
	strcpy(write_file_1_arg.buffer.buffer_val, buffer);

	//printf("Writing file... ");
	result_3 = write_file_1(&write_file_1_arg, clnt);
	//printf("DONE.\n");

	if (result_3 == (write_output*) NULL) {
		clnt_perror(clnt, "call failed");
	}

	printf("%s\n", result_3->out_msg.out_msg_val);
}


int main (int argc, char *argv[]) {
	char* host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}

	host = argv[1];
	ssnfsprog_1(host);

	user_name = getpwuid(getuid())->pw_name; // given in assignment spec

	// ==================================================
	// PASTE TESTING CODE BELOW HERE
	// ==================================================

	// TESTING SCRIPT GIVEN IN ASSIGNMENT
	// Spec: "code that I will use in the main() of your client program for testing is provided below"
	int i,j;
	int fd1,fd2;
	char buffer[100];
	fd1=Open("File1"); // opens the file "File1"
	for (i=0; i< 20;i++){
		Write(fd1, "This is a test program for cs570 assignment 4", 15);
	}
	Close(fd1);
	fd2=Open("File1");
	for (j=0; j< 20;j++){
		Read(fd2, buffer, 10);
		printf("%s\n",buffer);
	}
	Close(fd2);
	Delete("File1");
	List();

	// ==================================================
	// PUT TESTING CODE ABOVE HERE
	// ==================================================

	exit (0);
}
