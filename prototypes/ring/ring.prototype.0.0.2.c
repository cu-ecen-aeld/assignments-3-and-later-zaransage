#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#define BUFFER_SIZE = 10


typedef struct {
    int *buffer;
    int head;
    int tail;
    int count;
} CircularBuffer;


CircularBuffer *createCircularBuffer(){
    CircularBuffer * cb = (CircularBuffer *)malloc(sizeof(CircularBuffer));
    
    if (cb == NULL){
        return NULL;
    }

    cb->buffer = (int *)malloc(sizeof(int) * BUFFER_SIZE);
    if (cb->buffer == NULL){
        free(cb);
        return NULL;
    }

    cb->head = 0;
    cb->tail = 0;
    cb->count =0;

    return cb;
}

int main (){



    

    return 0;
}