#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4

void* say_hello(void* arg) {
    long id = (long)arg;
    printf("Hello from thread %ld\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    for (long i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, say_hello, (void*)i);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    printf("Tat ca thread da hoan thanh.\n");
    return 0;
}