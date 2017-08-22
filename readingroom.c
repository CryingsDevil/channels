#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <errno.h>
#include <string.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;  
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;  
// static pthread_mutex_t writerLocker = PTHREAD_MUTEX_INITIALIZER;
static int isWriterIn = 0;
static int peopleInRoom = 0;
// static int peopleWaiting = 0;
static int isWriterWait = 0;
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
    int success = -1;
    // pthread_mutex_t readerC;
    // int success = pthread_mutex_init(&readerC, NULL);
    // if(success < 0){
    //     printf("No enough memory for reader %d.", i.id);
    //     return NULL;
    // }

    // pthread_mutex_lock(&readerC);
    // success = -1;
    /* This line must be printed only if the reader has to wait. */
    if(isWriterIn == 1 || isWriterWait > 0){
        // while(success < 0)success = pthread_mutex_trylock(&mtx);
        pthread_mutex_lock(&mtx);
        // peopleWaiting++;
        printf("%i: Reader %i waiting ...\n", now(), i.id);
        
        while(isWriterIn == 1 || isWriterWait > 0){
            // printf("%d REader still waiting\n", i.id);
            pthread_cond_wait(&cond, &mtx);
        }
        pthread_mutex_unlock(&mtx); 
    }
    
    

    /* Print this line when the reader enters the room. */
    success = -1;
    // while(success < 0)success = pthread_mutex_trylock(&mtx);
    pthread_mutex_lock(&mtx);
    // peopleWaiting--;
    peopleInRoom++;
    printf("%i: Reader %i enters room.\n", now(), i.id);
    pthread_mutex_unlock(&mtx);
    

    /* Execute this line when it is safe to do so. */
    read_documents(i.delay, i.id);

    /* Print this line when the reader leaves the room. */
    success = -1;
    // while(success < 0)success = pthread_mutex_trylock(&mtx);
    pthread_mutex_lock(&mtx);
    peopleInRoom--;
    printf("%i: Reader %i leaves room.\n", now(), i.id);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);
    
    // pthread_mutex_unlock(&readerC);
    // if(pthread_mutex_destroy(&readerC) != 0){
    //     printf("Reader %i cannot destroy its mutex.\n", i.id);
    // }
    // printf("-----------------Reader  %d closed.\n", i.id);
    return NULL;
}

void* writer(void* p) {
    info* param = (info*) p;
    info i = *param;
    free(p);

    /* Print this line before the first synchronisation operation. */
    printf("%i: New writer %i (delay=%i).\n", now(), i.id, i.delay);
    int success = -1;
    // pthread_mutex_t writerLock;
    // int success = pthread_mutex_init(&writerLock, NULL);
    // if(success < 0){
    //     printf("No enough memory for writer %d.\n",i.id);
    //     return NULL;
    // }   
    // pthread_mutex_lock(&writerLock);
    /* Print this line only if the writer has to wait. */
    // success = -1;
    if(peopleInRoom > 0){
        // while(success < 0)success = pthread_mutex_trylock(&mtx);
        pthread_mutex_lock(&mtx);
        isWriterWait++;
        printf("%i: Writer %i waiting ...\n", now(), i.id);
        
        while(peopleInRoom != 0){
            // printf("%d Writer still waiting\n", i.id);
            pthread_cond_wait(&cond, &mtx);

        }
        pthread_mutex_unlock(&mtx);
    }
    
    

    /* Print this line when the writer enters the room. */
    success = -1;
    // while(success < 0)success = pthread_mutex_trylock(&mtx);
    pthread_mutex_lock(&mtx);
    peopleInRoom++;
    isWriterIn = 1;
    isWriterWait--;
    printf("%i: Writer %i enters room.\n", now(), i.id);
    pthread_mutex_unlock(&mtx);
    
    /* Execute this line when it is safe to do so. */
    write_documents(i.delay, i.id);
    success = -1;

    /* Print this line when the writer leaves the room. */
    // while(success < 0)success = pthread_mutex_trylock(&mtx);
    pthread_mutex_lock(&mtx);
    isWriterIn = 0;
    peopleInRoom--;
    printf("%i: Writer %i leaves room.\n", now(), i.id);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);
   
    // pthread_mutex_unlock(&writerLock);
    // if(pthread_mutex_destroy(&writerLock) != 0){
    //     printf("Writer %i cannot destroy its mutex.\n", i.id);
    // }
    // printf("---------------------------------------Writer %d closed.\n" , i.id);
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
    // // sleep(3);
    // int success = 0;
    // success = pthread_mutex_destroy(&mtx);
    // printf("%d\n", success);
    // if( success != 0)puts("Mutex mtx destroy failed!");
    // // printf("%d\n", pthread_mutex_destroy(&mtx));
    // if(pthread_cond_destroy(&cond) != 0)puts("Condition cond destroy failed!");
    // pthread_mutex_destroy(&mtx);
    // free(mtx);
    // pthread_cond_destroy(&cond);
    // free(cond);
    // while(pthread_mutex_destroy(&mtx) != 0)puts("Mutex destroyed failed.");
    printf("Mutex destroyed: %s\n",strerror(pthread_mutex_destroy(&mtx)));
    printf("Condition destroyed: %s\n",strerror(pthread_cond_destroy(&cond)));
    return 1;
    
}

