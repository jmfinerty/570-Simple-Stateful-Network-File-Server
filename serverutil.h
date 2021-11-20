// University of Kentucky CS570-001 "Modern Operating Systems", Fall 2021.
// Assignment 4: "Implement a Simple Stateful Network File Server"
// Due November 22nd, 2021 at 11:59PM.
// Work is original and by Jacob Finerty, student ID 912307438,
// with exception of provided sample code and files.
// Any sources referenced are cited in comments.
// Last modification before submission: 19 Nov 2021, 9:33PM

#ifndef SERVERUTIL_H
#define SERVERUTIL_H

#define true 1
#define false 0
typedef int bool;


#define LIST_DELIM      "\n" // delimiter used when printing List() output to user
#define OUT_MSG_BUF_LEN 1024 // buffer size for messages returned to user


// see comments in server.c
bool is_valid_file_descriptor();
bool is_valid_user_name();
bool is_valid_file_name();
int get_filetable_index_of_file_descriptor();
int get_filetable_index_of_file_name();
int get_filetable_index_of_user_file();
int get_num_users_in_di();
int get_num_user_files_in_di();
int get_di_index_of_file();
int get_di_index_of_user_name();
void print_read_pointer_pos_warning();

#endif /* SERVERUTIL_H */