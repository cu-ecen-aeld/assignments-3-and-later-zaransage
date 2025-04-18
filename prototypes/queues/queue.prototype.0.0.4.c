#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"
#include "shannon.c"


typedef struct slist_data_s slist_data_t;

struct slist_data_s {
    int value;
    SLIST_ENTRY(slist_data_s) entries;
};

static void myTime() {

    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

    char buffer[80];
    // Mon 02 Sep 2024 08:53:12 +0000
    strftime(buffer, sizeof(buffer), "%a %d %b %y %r %z", local);

    printf("%s\n", buffer);

}

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