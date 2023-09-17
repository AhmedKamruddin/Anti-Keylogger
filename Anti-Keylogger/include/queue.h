#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#define MAX_SIZE 10 // Maximum size of the circular queue

struct Queue;

// Function to create an empty circular queue
struct Queue* create_queue();

// Function to check if the circular queue is empty
bool is_empty_queue(struct Queue* queue);

// Function to check if the circular queue is full
bool is_full_queue(struct Queue* queue);

// Function to add an item to the circular queue (enqueue)
bool enqueue(struct Queue* queue, unsigned short value);

// Function to remove an item from the circular queue (dequeue)
unsigned short dequeue(struct Queue* queue);

// Function to peek at the rear element of the circular queue
unsigned short peek_from_rear(struct Queue* queue);

// Function to display the elements in the circular queue
void display_queue(struct Queue* queue);

// Function to free the memory used by the circular queue
void free_queue(struct Queue* queue);


extern struct Queue* key_queue;

#endif
