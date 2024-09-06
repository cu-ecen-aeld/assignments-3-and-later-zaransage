#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "shannon.c"


typedef struct slist_data_s slist_data_t;

struct slist_data_s {
    int value;
    SLIST_ENTRY(slist_data_s) entries;
};



int main (int argc, char *argv[]) {

  slist_data_t *datap=NULL;

  SLIST_HEAD(slisthead, slist_data_s) head;
  SLIST_INIT(&head);

  datap = malloc(sizeof(slist_data_t));
  datap->value = (int) (randshannon() * 1000.);
  SLIST_INSERT_HEAD(&head, datap, entries);

  datap = malloc(sizeof(slist_data_t));
  datap->value = (int) (randshannon() * 1000.);
  SLIST_INSERT_HEAD(&head, datap, entries);

  SLIST_FOREACH(datap, &head, entries) {
    printf("%d\n", datap->value);
  }

}