#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// Define the circular queue structure
struct Queue {
    unsigned short items[MAX_SIZE];
    int front;
    int rear;
};

// Function to create an empty circular queue
struct Queue* create_queue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    if (queue == NULL) {
        return NULL; // Memory allocation failed
    }
    
    queue->front = -1;
    queue->rear = -1;
    return queue;
}

// Function to check if the circular queue is empty
bool is_empty_queue(struct Queue* queue) {
    return (queue->front == -1 && queue->rear == -1);
}

// Function to check if the circular queue is full
bool is_full_queue(struct Queue* queue) {
    return ((queue->rear + 1) % MAX_SIZE == queue->front);
}

// Function to add an item to the circular queue (enqueue)
bool enqueue(struct Queue* queue, unsigned short value) {
    if (is_full_queue(queue)) {
        return false; // Queue is full
    }
    if (is_empty_queue(queue)) {
        queue->front = queue->rear = 0;
    } else {
        queue->rear = (queue->rear + 1) % MAX_SIZE;
    }
    queue->items[queue->rear] = value;
    return true;
}

// Function to remove an item from the circular queue (dequeue)
unsigned short dequeue(struct Queue* queue) {
    unsigned short item;
    if (is_empty_queue(queue)) {
        return 0; // Queue is empty
    } else if (queue->front == queue->rear) {
        item = queue->items[queue->front];
        queue->front = queue->rear = -1;
    } else {
        item = queue->items[queue->front];
        queue->front = (queue->front + 1) % MAX_SIZE;
    }
    return item;
}

// Function to display the elements in the circular queue
void display_queue(struct Queue* queue) {
    if (is_empty_queue(queue)) {
        printf("Queue is empty.\n");
        return;
    }
    printf("Queue elements: ");
    int i = queue->front;
    do {
        printf("%hu ", queue->items[i]);
        i = (i + 1) % MAX_SIZE;
    } while (i != (queue->rear + 1) % MAX_SIZE);
    printf("\n");
}

// Function to free the memory used by the circular queue
void free_queue(struct Queue* queue) {
    free(queue);
}
