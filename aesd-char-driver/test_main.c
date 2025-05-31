#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"
#include "aesd-circular-buffer.c"
#include <stdlib.h>
#include <stdio.h>

int main(){

    // Initialize my buffer
    struct aesd_circular_buffer *myBuffer = malloc(sizeof(struct aesd_circular_buffer)); 

    // init said buffer
    aesd_circular_buffer_init(myBuffer);

    // Set up an entry
    // Probably inside the funtion

    char *data = "Dirka";

    // This is really the init of the data block
    struct aesd_buffer_entry *entry = malloc(sizeof(struct aesd_buffer_entry));
    entry->buffptr = data;
    entry->size = strlen(data);
    // The data block init stops here

    printf("%s, %ld\n", entry->buffptr, entry->size);

    //return a position

    int offset = 4;

    printf("Char %c at offset %d.\n", entry->buffptr[offset], offset);

    // Add entry to buffer
    aesd_circular_buffer_add_entry(myBuffer, entry);

    // Position find entry offset

    size_t char_offset = 1;
    size_t *entry_offset_byte_rtn;

    aesd_circular_buffer_find_entry_offset_for_fpos(myBuffer, char_offset, entry_offset_byte_rtn);

    // Free Struct Memory from Heap
    free(entry);
    free(myBuffer);
    //free(my_offset_fpos);
    return 0;

 }