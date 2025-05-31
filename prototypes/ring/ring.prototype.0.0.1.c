#include <stdio.h>

#define BUFFER


// I need to get a structure to be the buffer.
// So let's say I have a list with 10 elements.

// buffer = [10];

// Then I have to say 'insert in available cell'
// 'IN' is a position I need to reference to 'insert' an item.
// 'OUT' is the position I can give a location to 'get' an item.

// My 'in' position needs to be stored in the queue or the object in the queue?

// Let me try having the buffer determine this.

typedef struct {
    const int *out;
    const int *in;
    int buffer[BUFFER];
} circular_buffer;


// So I get the number from the out column
// Then ask the buffer for that spot.
void get_data(circular_buffer *circular_buffer, (*) *data){
    int out = *circular_buffer->out;
    circular_buffer->buffer[out] = data;
}


// So I get data and add it.
// I know the number to insert because the buffer says
void put_data(circular_buffer *circular_buffer, (*) *data){
    // take data from outside
    // Insert it into the buffer
    // Increment the 'in' counter by 1
    // Once I get to the size of buffer +1
    // Increment the 'in' counter to first spot in buffer
    int in = *circular_buffer->in
    circular_buffer->buffer[circular_buffer->in] = data;
    
    circular_buffer->in += 1;
    circular_buffer->out += 1;
}


int main(int argc, char *argv[]){

    circular_buffer test_buffer;

    char value_one = "Test 1";

    put_data(&test_buffer, *value_one);
    //put_data(&test_buffer);

    return 0;
}