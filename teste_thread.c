#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *PrintHello( void *threadid) {
    long tid;
    char frase[30];
    tid = (long) threadid;
    scanf("%s",frase);
    printf("HELLO WOLD! ITS ME, THREAD #%ld\n", tid);
    printf("%s\n\n\n",frase);
    pthread_exit(NULL);
}
void *teste1(void *threadid) {
    int i;
    for(i = 0; i < 1000000000; i++) {

        if (i == 1000000000/2){
            printf("CHEGUEI");
        }
        if (i == 1000000000 - 1 ) {
            printf("CHEGUEI DE NOVO");
        }
    }
}
int main() {
    pthread_t thread1,thread2;
    int teste;
    long t;
    t = 1;
    printf("In main: creating thread 1\n");
    teste = pthread_create(&thread1, NULL, *teste1, (void *)t);
      if (teste){
         printf("ERROR; return code from pthread_create() is %d\n", teste);
         exit(-1);
      }
      t = 2;
    printf("In main: creating thread 2\n");
    teste = pthread_create(&thread2, NULL, PrintHello, (void *)t);
      if (teste){
         printf("ERROR; return code from pthread_create() is %d\n", teste);
         exit(-1);
      }
    pthread_exit(NULL);
    return 0;
}

