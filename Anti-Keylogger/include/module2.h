#ifndef MODULE2_H
#define MODULE2_H

#include <pthread.h>
#include "queue.h"

extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern int is_module1_done;

void* module2();

#endif