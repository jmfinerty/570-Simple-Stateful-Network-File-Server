#ifndef SERVERUTIL_H
#define SERVERUTIL_H

#define true 1
#define false 0
typedef int bool;


bool is_valid_file_descriptor();
bool is_valid_user_name();
bool is_valid_file_name();

int get_filetable_index_of_file_descriptor();
int get_usersblocks_index_of_user_name();


#endif /* SERVERUTIL_H */