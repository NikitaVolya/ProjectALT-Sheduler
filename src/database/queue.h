#ifndef _QUEUE_H_
#define _QUEUE_H_

#include<stdlib.h>
#include<stdio.h>

struct QueueElement {
    void *value;
    struct QueueElement *next, *prev;
};
typedef struct QueueElement QueueElement;


typedef struct {
    QueueElement *start, *end;
    size_t size; 

    void (*free_function)(void*);
} Queue;


typedef struct {
    QueueElement *current;
} QueueIterator;

/* ========================== */
/*       QUEUE FUNCTIONS      */
/* ========================== */
Queue* create_queue();

void free_queue(Queue *queue);

inline size_t get_queue_size(Queue *queue) {
    return queue->size;
}

void set_queue_element_free_function(Queue *queue, void (*free_function)(void*));

Queue* push_queue_element(Queue *queue, void *value);

void* pop_queue_element(Queue *queue);

void* get_queue_element(Queue *queue, size_t index);

void* queue_find_first_element(Queue *queue, int (*predicate)(void*));

void* create_queue_copy(Queue *queue, void* (*element_copy_f)(void*));

Queue* queue_filter(Queue *queue, int (*predicate)(void*));

/* ========================== */
/*  QUEUE ITERATOR FUNCTIONS  */
/* ========================== */
int queue_iterator_is_end(QueueIterator *iterator);

QueueIterator* get_queue_iterator(Queue* queue);

QueueIterator* get_queue_iterator_backward(Queue* queue);

QueueIterator* queue_iterator_forward(QueueIterator* iterator);

QueueIterator* queue_iterator_backward(QueueIterator* iterator);

void* get_queue_iterator_value(QueueIterator* iterator);

inline void close_queue_iterator(QueueIterator* iterator) {
    free(iterator);
}

#endif /* _QUEUE_H_ */