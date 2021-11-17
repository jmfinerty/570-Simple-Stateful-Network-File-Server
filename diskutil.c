#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "diskutil.h"


FileTable* ft = NULL;


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

int initialize_virtual_disk() {
    if (ft == NULL) {
        ft = _initialize_file_table();
    }

    // check if vdisk exists. F_OK = check existence
    // https://linux.die.net/man/2/access
    if (access(VDISK_LOC, F_OK)) {
        // access returned 1
        // do something to initialize disk for the first time
    } else {
        FILE* vdisk = fopen(VDISK_LOC, "r");
        // access returned 0
        // do something to read existing disk into memory
    }

    return 1;

}