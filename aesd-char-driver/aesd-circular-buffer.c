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
     if (!buffer->full) {
        return NULL;
     }

     // I need something to hold the place of our offset in the loop...
     size_t my_new_offset = 0;

     // I somehow need total value savailable and I wonder if I need that size math...
     size_t buffer_size = sizeof(buffer->entry) / sizeof(buffer->entry[0]);
     
     // See if this error is a result of changes in buffer size: ( I dont want to use stdbool.h)
     // I read that kernel development hates that and so now I will try ternary.

    size_t my_entry_check = buffer->full ? buffer_size : buffer->in_offs;

    // for every entry in the buffer, give me the nth entry
    size_t entry_position = buffer->out_offs;
    for (size_t i = 0; i < my_entry_check; i++){
     struct aesd_buffer_entry *entry = &buffer->entry[entry_position];
     if (!entry->buffptr || entry->size == 0){
        return NULL;
     }

     if (char_offset >= my_new_offset && char_offset < (my_new_offset + entry->size)){
        *entry_offset_byte_rtn = (char_offset - my_new_offset);
        return entry;
     }

     // Update the ENTRY POSITION NOT THE OFFSET DIRECTLY  
     my_new_offset += entry->size;
     entry_position = (entry_position + 1) % buffer_size; 

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

     // Let me get the size for my later modulus
     int buffer_size = sizeof(buffer->entry) / sizeof(buffer->entry[0]);

     // What about if we are already full?
     // I may have to break this into two
     if ( buffer->full && buffer->in_offs == buffer->out_offs){
      buffer->out_offs = (buffer->out_offs +1) % buffer_size;  
     }

     // Add the entry
     buffer->entry[buffer->in_offs] = *add_entry;

     // Now lets see about advancing this by using the size of the array
     buffer->in_offs = (buffer->in_offs +1) % buffer_size;

     // Separate check for buffer full status
     buffer->full = (buffer->in_offs == buffer->out_offs);

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