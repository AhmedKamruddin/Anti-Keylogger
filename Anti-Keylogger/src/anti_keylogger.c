#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "module1.h"
#include "module2.h"
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int is_module1_done = 0;

struct Queue* key_queue = NULL;

int main()
{
    key_queue = create_queue();
    pthread_t thread_id1, thread_id2;
    pthread_create(&thread_id1, NULL, module1, NULL);
    pthread_create(&thread_id2, NULL, module2, NULL);
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
}

// int main()
// {
//     module1();
// }