#include <stdio.h>
#include <stdlib.h>


struct First{
    char letter;
    int number;
};


struct Second {
    char letter;
    int number;
};


struct Alpha {
    struct First first;
    struct Second second;
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

    printf("%c, %d\n", f->letter, f->number);


    struct Alpha *a = malloc(sizeof(struct Alpha));

    if (!a) {
        free(f);
        free(s);
        free(a);
    }

    a->first.letter = 'A';
    a->second.number = 20;

    printf("%c, %d\n", a->first.letter, a->second.number);

    free(f);
    free(s);
    free(a);

    return 0;
}