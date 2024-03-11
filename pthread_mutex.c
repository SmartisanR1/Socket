#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int s = 0;

void *myfunc(void *args)
{
    int i = 0;

    pthread_mutex_lock(&lock);
    for (i = 0; i < 1000000; i++)
    {
        s++;
    }
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;
    int i;

    // pthread_mutex_init(&lock, NULL);

    pthread_create(&tid1, NULL, myfunc, NULL);
    pthread_create(&tid2, NULL, myfunc, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("s = %d\n", s);
    return 0;
}