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

     // Check for inputs
     if (!buffer) {
      return NULL;
     }

     if (!entry_offset_byte_rtn){
      return NULL;
     }

     // Okay, is the buffer empty...
     // Check for empty - I might wanto to break these up.
     if (!buffer->full && buffer->in_offs == buffer->out_offs) {
        return NULL;
     }

     // Okay I need to get the oldest starting point from buff-out_offs.
     // Placeholder

     // I somehow need total value savailable and I wonder if I need that size math...
     int buffer_size = sizeof(buffer->entry) / sizeof(buffer->entry[0]);


     // for every entry in the buffer, give me the nth entry
     // Placeholder

     for (size_t i = 0; i < buffer_size; i++){
      struct aesd_buffer_entry *entry = &buffer->entry[buffer->out_offs];

      if (!entry->buffptr || entry->size == 0){
         return NULL;
      }

     // Get the actual bufferptf from the cell in the buffer itself, specified by the char offset.

     //Somehow, read into the returned bufferptr at the position given by entry_offset_byte_rtn
     // Placeholder

     }

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

     // Check for parts being satisfied
     if (!buffer){
      return;
     }

     if (!add_entry){
      return;
     }

     if (!add_entry->buffptr){
      return;
     }

     // I wonder if I can use this to avoid modulous
     // I may not need this actually
     int buffer_size = sizeof(buffer->entry) / sizeof(buffer->entry[0]);

     // What about if we are already full?
     if ( buffer->out_offs == (buffer->in_offs) ){
        buffer->full = true;
     }

     // Now lets see about advancing this by using the size of the array
     if ( (buffer->in_offs + 1) >=  buffer_size){
      buffer->in_offs = 0;
   }

     // Add the entry
     buffer->entry[buffer->in_offs] = *add_entry;
     buffer->in_offs++;

     printf("Current in offset for buffer is: %d\n", buffer->in_offs);
     printf("Current out offset for buffer is: %d\n", buffer->out_offs);
     printf("Current size of the buffer is: %i\n", buffer_size);

 }
 
 /**
 * Initializes the circular buffer described by @param buffer to an empty struct
 */
 void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
 {
     // Check a condition where somehow I do not have a buffer...
     if (!buffer){
      return;
     }

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