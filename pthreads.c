#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 4

void increment_count(){
 pthread_mutex_lock(&count_mutex);
 count = count + 1;
 pthread_mutex_unlock(&count_mutex);
}

long get_count(){
 long c;
 pthread_mutex_lock(&count_mutex);
 c = count;
 pthread_mutex_unlock(&count_mutex);
 return (c);
}

void *PrintHello(void *threadId) {
    int i;
    long tId;
    tId = (long)threadId;
    printf("Hello World! It's thread #%ld!\n", tId);
    for (i=0;i<1000000000;i++){}
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_mutex_t count_mutex;
    long count;
    pthread_t threads[NUM_THREADS];
    int ret, i;
    long t;
    for(t=0;t<NUM_THREADS;t++){
        printf("In main: creating thread %ld\n", t);

        ret = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
        if (ret){
            printf("ERROR! Return code: %d\n", ret);
            exit(-1);
        }
    }
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("\n\nACABOU\n\n");
    pthread_exit(NULL);
}
