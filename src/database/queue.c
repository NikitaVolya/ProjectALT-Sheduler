#include"queue.h"


/* ========================== */
/*       QUEUE FUNCTIONS      */
/* ========================== */
Queue* create_queue() {
    Queue *res;

    if ((res = (Queue*) malloc(sizeof(Queue))) == NULL) {
        fprintf(stderr, "Error while queue memory alocation\n");
        return NULL;
    }

    res->size = 0;
    res->start = NULL;
    res->end = NULL;

    return res;
}

void free_queue(Queue *queue) {
    QueueElement *tmp, *curr;

    curr = queue->start;

    while (curr != NULL) {

        free(curr->value);
        
        tmp = curr->next;
        free(curr);

        curr = tmp;
    }
    free(queue);
}

Queue* push_queue_element(Queue* queue, void *value) {
    QueueElement *new_element, *tmp;

    if ((new_element = (QueueElement*) malloc(sizeof(QueueElement))) == NULL) {
        fprintf(stderr, "Error while queue element memory alocation\n");
        return NULL;
    }

    new_element->value = value;
    new_element->next = NULL;
    new_element->prev = NULL;

    if (queue->end == NULL) {
        queue->start = new_element;
        queue->end = new_element;
    } else {
        tmp = queue->end;

        tmp->next = new_element;
        new_element->prev = tmp;

        queue->end = new_element;
    }

    queue->size++;
    return queue;
}


void* pop_queue_element(Queue* queue) {
    void *res;
    QueueElement *tmp;

    if (queue == NULL || queue->start == NULL) {
        fprintf(stderr, "Queue is NULL or clear\n");
        return NULL;
    }

    tmp = queue->start;
    res = tmp->value;
    
    if (queue->start == queue->end) {
        queue->start = NULL;
        queue->end = NULL;
    } else {
        queue->start = tmp->next;
    }

    free(tmp);
    return res;
}


void* get_queue_element(Queue *queue, size_t index) {
    QueueElement *c;

    if (index >= queue->size) {
        fprintf(stderr, "Queue index out of range\n");
        exit(EXIT_FAILURE);
    }

    c = queue->start;
    while (index != 0) {
        c = c->next;
        index--;
    }

    return c->value;
}

void* queue_find_first_element(Queue* queue, int (*predicate)(void*)) {
    QueueElement *current;

    current = queue->start;
    while (current != NULL && !predicate(current->value)) {
        current = current->next;
    }

    return (current == NULL ? NULL : current->value);
}


/* ========================== */
/*  QUEUE ITERATOR FUNCTIONS  */
/* ========================== */

int queue_iterator_is_end(QueueIterator *iterator) {
    if (iterator == NULL) {
        fprintf(stderr, "Iterator is NULL\n");
        return 1;
    }
    return iterator->current == NULL;
}

QueueIterator* get_queue_iterator(Queue* queue) {
    QueueIterator* res;

    if (queue == NULL) {
        fprintf(stderr, "Error while execution get_queue_iterator, queue is NULL\n");
        return NULL;
    }

    if ((res = (QueueIterator*) malloc(sizeof(QueueIterator))) == NULL) {
        fprintf(stderr, "Error while queue iterator memory alocation\n");
        return NULL;
    }

    res->current = queue->start;
    return res;
}

QueueIterator* queue_iterator_forward(QueueIterator* iterator) {
    if (queue_iterator_is_end(iterator)) {
        exit(EXIT_FAILURE);
    }
    iterator->current = iterator->current->next;
    return iterator;
}

QueueIterator* queue_iterator_backward(QueueIterator* iterator) {
    if (queue_iterator_is_end(iterator)) {
        exit(EXIT_FAILURE);
    }
    iterator->current = iterator->current->prev;
    return iterator;
}

void* get_queue_iterator_value(QueueIterator* iterator) {
    if (queue_iterator_is_end(iterator)) {
        exit(EXIT_FAILURE);
    }
    return iterator->current->value;
}
