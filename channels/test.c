#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "channels.h"


/*
 * The 5-stage pipeline demo.
 */

struct item {
    int id;
    int stage;
};

enum messages {
    MSG_ITEM,
    MSG_DONE
};

typedef struct {
    enum messages type;
    struct item item;
} message;

typedef struct {
    int n_items;
    int stage;
    int id;
} thread_info;

int main(int argc, char** argv) {
    time_t start_time; time(&start_time);
    srand((unsigned) start_time);
    int e = ch_setup(); if (e < 0) { puts("setup failed"); return 1; }
	message *mr;
    message *m = malloc(sizeof(message));
    m->type = MSG_ITEM;
    m->item.id = 1;
	m->item.stage = 1;
    ch_send(1, m);
    puts("m1 sent");

    message *m2 = malloc(sizeof(message));
    m2->type = MSG_ITEM;
    m2->item.id = 222;
	m2->item.stage = 1;
	int err = 0;
	err++;
	// err = ch_recv(1,&mr);
    ch_send(1, m2);
    puts("m2 sent");
   	err = ch_recv(1,(void**) &mr);
   	// err = ch_recv(1,(void**) &mr);
   	printf("id is %d\n",m->item.id);


}