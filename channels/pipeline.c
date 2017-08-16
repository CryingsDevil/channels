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

struct timespec spec;
double offset() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double o = (now.tv_sec - spec.tv_sec) + ((double) now.tv_nsec - spec.tv_nsec) / 1.0e9;
    return o;
}

void* worker(void* param) {
    thread_info *info = (thread_info*) param;
    int err;
    int done = 0;
    int items = 1;
    
    printf("[%0.6f] This is thread %i (stage %i).\n", offset(), info->id, info->stage);
    if (info->stage == 1) {
        printf("[%0.6f] I am going to produce %i items.\n", offset(), info->n_items);
    } else if (info->stage == 5) {
        printf("[%0.6f] I am going to consume %i items.\n", offset(), info->n_items);
    }

    while (!done) {
        if (info->stage == 1) {
            /* produce items */
            usleep((1 + random() % 5) * 100 * 1000);
            message *m = malloc(sizeof(message));
            if (m == NULL) {
                puts("Out of memory!");
                abort();
            }
            m->type = MSG_ITEM;
            m->item.id = items;
            m->item.stage = 1;
            printf("[%0.6f] Thread %i has produced item #%i.\n", offset(), info->id, items);
            items++;
            err = ch_send(2, m);
            if (err) {
                puts("Send error.");
                abort();
            }
            if (items > info->n_items) {
                printf("[%0.6f] Thread %i is done producing items.\n", offset(), info->id);
                done = 1;
            }
        } else {
            /* since we're not the producer, fetch items. */
            message *m;
            printf("[%0.6f] Thread %i waiting to receive.\n", offset(), info->id);
            err = ch_recv(info->stage, (void**) &m);
            if (err) {
                puts("Recv error.");
                abort();
            }
            if (m->type == MSG_DONE) {
                printf("[%0.6f] Thread %i got done message.\n", offset(), info->id);
                done = 1;
                free(m);
            } else if (m->type == MSG_ITEM) {
                if (info->stage == 5) {
                    /* consume item */
                    printf("[%0.6f] Thread %i is consuming item %i (stage %i).\n",
                        offset(), info->id, m->item.id, m->item.stage);
                        usleep(200 * 1000);
                    free(m);
                    items++;
                    if (items > info->n_items) {
                        printf("[%0.6f] Thread %i has consumed all items.\n",
                            offset(), info->id);
                        message *m = malloc(sizeof(message));
                        if (m == NULL) {
                            puts("Out of memory.");
                            abort();
                        }
                        m->type = MSG_DONE;
                        err = ch_send(0, m);
                        if (err) {
                            puts("Send error.");
                            abort();
                        }
                    }
                } else {
                    /* upgrade item and pass it on. */
                    printf("[%0.6f] Thread %i is upgrading item %i (stage %i).\n",
                        offset(), info->id, m->item.id, m->item.stage);
                    usleep((2 + random() % 3) * 100 * 1000);
                    m->item.stage++;
                    err = ch_send(info->stage + 1, m);
                    if (err) {
                        puts("Send error.");
                        abort();
                    }
                }
            } else {
                printf("Unexpected message: %i\n", m->type);
                abort();
            }
        }
    }
    printf("[%0.6f] Thread %i is done.\n", offset(), info->id);
    return NULL;
}

int main(int argc, char** argv) {
    time_t start_time; time(&start_time);
    srand((unsigned) start_time);
    int e = ch_setup(); if (e < 0) { puts("setup failed"); return 1; }

    pthread_t threads[7];
    thread_info info[7] = {
    /*    n_items  stage  id */
        {      15,     1,  1 },
        {       0,     2,  2 },
        {       0,     3,  3 },
        {       0,     3,  4 },
        {       0,     4,  5 },
        {       0,     4,  6 },
        {      15,     5,  7 }
    };

    puts("Running ...");
    clock_gettime(CLOCK_REALTIME, &spec);
    int err;
    for (int i = 0; i < 7; i++) {
        err = pthread_create(threads + i, NULL, worker, info + i);
        if (err) { puts("Failed to create thread."); return 1; }
    }

    message *done_message;
    err = ch_recv(0, (void**) &done_message);
    if (err) { puts("Failed to listen on DONE channel."); return 1; }
    if (done_message->type != MSG_DONE) {
        printf("Expected 'done', got %i\n", done_message->type);
        return 7;
    }
    free(done_message);    

    /* tell everyone in the later stages to finish */
    printf("[%0.6f] main thread: cleanup time.\n", offset());
    for (int i = 0; i < 7; i++) {
        if (info[i].stage > 1) {
            done_message = malloc(sizeof(message));
            if (done_message == NULL) {
                puts("Out of memory while cleaning up.");
                return 1;
            }
            done_message->type = MSG_DONE;
            /* There is no guarantee that the message will go to the 'intended'
               thread. What we do know is that each stage will get the correct
               number of messages so everyone ends up terminating.
            */
            printf("[%0.6f] main thread sending 'done' on channel %i.\n", offset(), info[i].stage);
            err = ch_send(info[i].stage, done_message);
            if (err) {
                puts("Error sending done message.");
                return 1;
            }
        }
    }

    for (int i = 0; i < 7; i++) {
        err = pthread_join(threads[i], NULL);
        if (err) {
            puts("Join error.");
            return 1;
        }
    }
    
    printf("[%0.6f] Done.\n", offset());
    return 0;
}
