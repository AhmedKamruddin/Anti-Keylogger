#ifndef MODULE1_H
#define MODULE1_H

#include <pthread.h>

extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern int is_module1_done;

void* module1();

#endif