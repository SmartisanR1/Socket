#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int arr[5000];
int s = 0;
typedef struct {
    int first;
    int last;
}MY_ARGS;

void *myfunc(void *args)
{
    int i = 0;
    MY_ARGS *myargs = (MY_ARGS *)args;

    for (i = myargs->first; i < myargs->last; i++)
    {
        s = s + arr[i];
    }

    printf("s = %d\n", s);
    s = 0;

    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;
    int i;

    for (i = 0; i < 5000; i++)
    {
        arr[i] = rand() % 50;
    }

    MY_ARGS args1 = {0, 2500};
    MY_ARGS args2 = {2500, 5000};

    pthread_create(&tid1, NULL, myfunc, &args1);
    pthread_create(&tid2, NULL, myfunc, &args2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("Main thread exiting\n");
    return 0;
}
