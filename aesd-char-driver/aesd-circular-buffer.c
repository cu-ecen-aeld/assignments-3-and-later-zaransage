/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

 #ifdef __KERNEL__
 #include <linux/string.h>
 #else
 #include <string.h>
 #endif
 
 #include "aesd-circular-buffer.h"
 #include <stdlib.h>
 #include <stdio.h>

//
 /**
  * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
  * @param char_offset the position to search for in the buffer list, describing the zero referenced
  *      character index if all buffer strings were concatenated end to end
  * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
  *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
  *      in aesd_buffer.
  * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
  * NULL if this position is not available in the buffer (not enough data is written).
  */
 struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer, size_t char_offset, size_t *entry_offset_byte_rtn )
 {
     /**
     * TODO: implement per description
     */

     // I get all elements of buffer
     // buffer
     //  . buffer_entry
     //    . pointer (called *buffptr)
     //    . size

     //  . in_offs
     //  . out_offs
     //  . full 


     // I get a single value of the char offset
     // char_offset

     // I get a single value of the entry offset
     // entry_offset_byte_rtn

     return NULL;
 }
 
 /**
 * Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
 * If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
 * new start location.
 * Any necessary locking must be handled by the caller
 * Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
 */
 void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
 {
     /**
     * TODO: implement per description
     */

     // I get all elements of buffer
     // buffer
     //  . buffer_entry
     //    . pointer (called *buffptr) entry[]
     //    . size

     //  . in_offs
     //  . out_offs
     //  . full 

     // I get a single value of the char offset
     // char_offset

     // I get a single value of the entry offset
     // entry_offset_byte_rtn

     // add_entry
     //  . pointer
     //  . size

     int buffer_size = sizeof(buffer->entry) / sizeof(buffer->entry[0]);

     if ( (buffer->in_offs + 1) >  buffer_size){
        buffer->in_offs = 0;
     }

     if ( buffer->out_offs == buffer->in_offs && sizeof(buffer->out_offs) && sizeof(buffer->in_offs) == 0){
        buffer->full = false;
     }

     if ( buffer->out_offs = (buffer->in_offs -1) ){
        buffer->full = true;
     }

     buffer->entry[buffer->in_offs] = *add_entry;
     buffer->in_offs++;

     printf("Current in offset for buffer is: %d\n", buffer->in_offs);
     printf("Current out offset for buffer is: %d\n", buffer->out_offs);
     printf("Current size of the buffer is: %i\n", buffer_size);
     printf("Current value of buffer entry is: %i\n", buffer->entry[0]);



 }
 
 /**
 * Initializes the circular buffer described by @param buffer to an empty struct
 */
 void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
 {
     memset(buffer,0,sizeof(struct aesd_circular_buffer));
 }




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
    entry->size = sizeof(data);
    // The data block init stops here

    printf("%s, %ld\n", entry->buffptr, entry->size);

    //return a position

    int offset = 4;

    printf("Char %c at offset %d.\n", entry->buffptr[offset], offset);

    // Add entry to buffer

    aesd_circular_buffer_add_entry(myBuffer, entry);


    // Free Struct Memory from Heap
    free(entry);
    free(myBuffer);
    return 0;

 }