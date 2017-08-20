#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "channels.h"

/*
 * Demo of a simple producer-consumer situation.
 */

enum channel_names {
    ITEM_CHANNEL=1,
    DONE_CHANNEL=0
};

enum messages {
    MSG_ITEM,
    MSG_DONE
};

typedef struct {
    enum messages type;
    int item_number;
} message;

const int N_ITEMS = 10;


void* producer(void* param) {
    int err;
    printf("Producer is ready.\n");

    for (int i = 0; i < N_ITEMS; i++) {
        /* spend some time "producing" */
        err = usleep((i + 1) * 100 * 1000);
        if (err) {
            printf("usleep failed: %s\n", strerror(errno));
            abort();
        }
        message *message = malloc(sizeof(message));
        if (message == NULL) { puts("Out of memory"); abort(); }
        message->type = MSG_ITEM;
        message->item_number = i + 1;
        printf("Item %i from producer is finished.\n", i+1);

        /* add it to the channel */
        err = ch_send(ITEM_CHANNEL, message);
        if (err) { puts("Failed to add item"); abort(); }
    }

    /* Tell the main thread that we are done. */
    message *message = malloc(sizeof(message));
    if (message == NULL) { puts("Out of memory"); abort(); }
    message->type = MSG_DONE;
    err = ch_send(DONE_CHANNEL, message);
    if (err) {
        puts("Failed to send shut down message.");
        abort();
    }
    
    printf("Producer done.\n");
    return NULL;
}

void* consumer(void* param) {
    message *message;
    puts("Consumer is ready.");

    for (;;) {
        int err = ch_peek(ITEM_CHANNEL);
        if (err < 0) {
            puts("Peek error.");
            abort();
        } else if (err == 0) {
            printf("Consumer: looks like we have to wait.\n");
        } else {
            printf("Consumer: looks like we have a message.\n");
        }
        
        err = ch_recv(ITEM_CHANNEL, (void**) &message);
        if (err < 0) { puts("Error receiving message."); abort(); }
        
        if (message->type == MSG_ITEM) {
            printf("Consumer is consuming item %i from producer.\n", message->item_number);
            usleep(300*1000);
            free(message);
        } else if (message->type == MSG_DONE) {
            printf("Consumer: all items consumed.\n");
            free(message);
            return NULL;
        }
    }
}

int main(int argc, char** argv) {
    int e = ch_setup(); if (e < 0) { puts("setup failed"); return 1; }

    pthread_t producer_thread;
    pthread_t consumer_thread;

    puts("Running ...");
    int err = pthread_create(&producer_thread, NULL, producer, NULL);
    if (err) { puts("Failed to create producer."); return 1; }
    err = pthread_create(&consumer_thread, NULL, consumer, NULL);
    if (err) { puts("Failed to create consumer."); return 1; }

    message *done_message;
    err = ch_recv(DONE_CHANNEL, (void**) &done_message);
    if (err) { puts("Failed to listen on DONE channel."); return 1; }
    
    err = pthread_join(producer_thread, NULL);
    if (err) { return 1; }
    
    /* the message belongs to us, so we can send it on */
    err = ch_send(ITEM_CHANNEL, done_message);
    if (err) { return 1; }
    
    err = pthread_join(consumer_thread, NULL);
    if (err) { return 1; }
    
    puts("Done.");
    return 0;
}
