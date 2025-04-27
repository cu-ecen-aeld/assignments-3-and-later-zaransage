#include <stdio.h>
#include <stdlib.h>



struct alpha {
    struct First first;
    struct Second second;
};


struct First{
    char letter;
    int number;
};


struct Second {
    char letter;
    int number;
};


void merge_two_structs(struct First *first, struct Second *second){

    first->letter = second->letter;
    first->number = second->number;

}


int main(){

    struct First *f = malloc(sizeof(struct First));
    struct Second *s = malloc(sizeof(struct Second));

    if (!f || !s) {
        free(f);
        free(s);
        return 1;
    }

    f->letter = 'D';
    f->number = 39;

    s->letter = 'M';
    s->number = 40;

    merge_two_structs(f, s);

    printf("%c, %d", f->letter, f->number);

    free(f);
    free(s);
    return 0;
}