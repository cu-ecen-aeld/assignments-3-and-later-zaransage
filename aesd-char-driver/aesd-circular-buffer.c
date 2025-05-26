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

     // I need something to hold the place of our offset in the loop...

     size_t my_new_offset = 0;

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
     // char_offset is 5 and my_new_offset is 5
     // or
     // char_offset is less than my_new_offset at 5
     // Something something darkside ...
     // entry->size ..... lets say equals 2 oh! Equals 1.
     // if char_offset is 5 and my_new_offset is 5 and my char_offset is less than 5 + 1 (or maybe entry size)

     // Then I would say starting at my char_offset until my_new_offset and my_new_offset is less than the entry size
     // I should be in the boundary of the memory space I can read if I delta the results.

     if (char_offset >= my_new_offset && char_offset < (my_new_offset + entry->size)){
        *entry_offset_byte_rtn = (char_offset - my_new_offset);
        return entry;
     }

     // I might be stuck with modulus if my other ideas do not work.     
     my_new_offset += entry->size;
     buffer->out_offs = (buffer->out_offs + 1) % buffer_size; 

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