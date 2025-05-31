#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "shannon.c"


typedef struct slist_data_s slist_data_t;
struct slist_data_s {
    int value;
    SLIST_ENTRY(slist_data_s) entries;
};


void slist(int n) {
  int i=0;
  slist_data_t *datap=NULL;

  SLIST_HEAD(slisthead, slist_data_s) head;
  SLIST_INIT(&head);

  for (i=0; i<n; i++) {
    datap = malloc(sizeof(slist_data_t));
    datap->value = (int) (randshannon() * 1000.);
    printf("Insert: %d\n", datap->value);
    SLIST_INSERT_HEAD(&head, datap, entries);
  }

  printf("\n");

  SLIST_FOREACH(datap, &head, entries) {
    printf("Read1: %d\n", datap->value);
  }

  printf("\n");

  while (!SLIST_EMPTY(&head)) {
    datap = SLIST_FIRST(&head);
    printf("Read2: %d\n", datap->value);
    SLIST_REMOVE_HEAD(&head, entries);
    free(datap);
  }

}


int main  {(int argc, char *argv[])
    if (argc !=2) {
        printf("Usage: %s <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("### SLIST ###\n");
    slist(atol(argv[1]));
    printf("\n\n");

}