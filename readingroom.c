#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

/* Do not change this code. */

struct timeval time0;

int now() {
    struct timeval _now;
    gettimeofday(&_now, NULL);
    return (_now.tv_sec - time0.tv_sec) * 100 +
           (_now.tv_usec - time0.tv_usec) / 10000;
}

void wait(int delay) {
    struct timespec p, q;
    delay *= 10;
    q.tv_sec = delay/1000;
    q.tv_nsec = 1000000 * (delay % 1000);
    int err = nanosleep(&q, &p);
    if (err) { abort(); }
}

void read_documents(int delay, int id) {
    printf("%i: Reader %i reading ...\n", now(), id);
    wait(delay);
    printf("%i: Reader %i done reading.\n", now(), id);
}

void write_documents(int delay, int id) {
    printf("%i: Writer %i writing ...\n", now(), id);
    wait(delay);
    printf("%i: Writer %i done writing.\n", now(), id);
}

typedef struct _info {
    int id;
    int delay;
    pthread_t *thread;
} info;

/* End of code to leave alone. */

void* reader(void* p) {
    info *param = (info*) p;
    info i = *param;
    free(p);

    /* Print this line when a new reader is created, before any synchronisation operations. */
    printf("%i: New reader %i (delay=%i).\n", now(), i.id, i.delay);

    /* This line must be printed only if the reader has to wait. */
    printf("%i: Reader %i waiting ...\n", now(), i.id);

    /* Print this line when the reader enters the room. */
    printf("%i: Reader %i enters room.\n", now(), i.id);

    /* Execute this line when it is safe to do so. */
    read_documents(i.delay, i.id);

    /* Print this line when the reader leaves the room. */
    printf("%i: Reader %i leaves room.\n", now(), i.id);

    return NULL;
}

void* writer(void* p) {
    info* param = (info*) p;
    info i = *param;
    free(p);

    /* Print this line before the first synchronisation operation. */
    printf("%i: New writer %i (delay=%i).\n", now(), i.id, i.delay);

    /* Print this line only if the writer has to wait. */
    printf("%i: Writer %i waiting ...\n", now(), i.id);

    /* Print this line when the writer enters the room. */
    printf("%i: Writer %i enters room.\n", now(), i.id);

    /* Execute this line when it is safe to do so. */
    write_documents(i.delay, i.id);

    /* Print this line when the writer leaves the room. */
    printf("%i: Writer %i leaves room.\n", now(), i.id);

    return NULL;
}

int main(void) {
    /* You may add code of your own to main() if you want to, but this should not be necessary.
     * Do not change the code that is already here.
     * Note that currently the number of threads is 100 and the writer rate is 1/8.
     * I may try out other values too while marking your assignment.
     */

    int count = 0;
    int seed = time(NULL) % 65536;
    printf("Random seed: %i.\n", seed);
    srand(seed);
    gettimeofday(&time0, NULL);

    pthread_t threads[100];
    for (int i = 0; i < 100; i++) {
        info *info = malloc(sizeof(info));
        if (info == NULL) {
            abort();
        }

        info->id = ++count;
        info->delay = (random() % 6) + (random() % 6) + 2;
        int isWriter = (random() % 8) == 0;
        printf("id=%i isWriter = %i\n", i, isWriter);
        info->thread=&threads[i];
        int err = pthread_create(&threads[i], NULL, isWriter ? writer : reader, info);
        if (err) { abort(); }

        wait (random() % 5);
    }

    for (int i = 0; i < 100; i++) {
        int err = pthread_join(threads[i], NULL);
        if (err) { abort(); }
    }
}

