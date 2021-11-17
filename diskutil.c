#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "diskutil.h"


FileTable* _initialize_file_table() {
    FileTable* table = (FileTable*) malloc(sizeof(FileTable)+1);

    // fill each file in table with default values
    for (int i = 0; i < MAX_FT_SIZE; i++) {
        strcpy(table->entry[i].ownerUserName, DEFAULT_USER_NAME);
        strcpy(table->entry[i].fileName, DEFAULT_FILE_NAME);
        table->entry[i].fileDescriptor = 0;
        table->entry[i].filePointerPos = 0;
    }

    return table;
}