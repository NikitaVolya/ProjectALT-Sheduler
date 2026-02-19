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

    res->free_function = &free;

    return res;
}

void free_queue(Queue *queue) {
    QueueElement *tmp, *curr;

    curr = queue->start;

    while (curr != NULL) {

        if (curr->value != NULL)
            queue->free_function(curr->value);

        tmp = curr->next;
        free(curr);

        curr = tmp;
    }
    free(queue);
}

void set_queue_element_free_function(Queue *queue, void (*free_function)(void*)) {
    if (free_function == NULL) {
        fprintf(stderr, "free_function is NULL\n");
        exit(EXIT_FAILURE);
    }
    queue->free_function = free_function;
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

void* create_queue_copy(Queue *queue, void* (*element_copy_f)(void*)) {
    Queue *res;
    QueueIterator *it;
    void *new_element, *old_element;

    if ((res = create_queue()) == NULL) {
        return NULL;
    }

    for (it = get_queue_iterator(queue); !queue_iterator_is_end(it); it = queue_iterator_forward(it)) {

        old_element = get_queue_iterator_value(it);
        new_element = element_copy_f(old_element);

        push_queue_element(res, new_element);
    }

    return res;
}

Queue* queue_filter(Queue *queue, int (*predicate)(void*)) {
    QueueElement *prev, *curr, *tmp;

    prev = NULL;
    curr = queue->start;

    do {
        if (predicate(curr->value)) {

            if (prev == NULL) {
                queue->start = curr;
                curr->prev = NULL;
            } else {
                curr->prev = prev;
                prev->next = curr;
            }

            prev = curr;
            queue->end = curr;

            curr = curr->next;
            prev->next = NULL;
        } else {
            tmp = curr;
            curr = curr->next;

            if (tmp->value != NULL)
                queue->free_function(tmp->value);
            free(tmp);
        }

    } while (curr != NULL);

    if (prev == NULL) {
        queue->start = NULL;
        queue->end = NULL;
    }

    return queue;
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

QueueIterator* get_queue_iterator_backward(Queue* queue) {
    QueueIterator* res;

    if (queue == NULL) {
        fprintf(stderr, "Error while execution get_queue_iterator, queue is NULL\n");
        return NULL;
    }

    if ((res = (QueueIterator*) malloc(sizeof(QueueIterator))) == NULL) {
        fprintf(stderr, "Error while queue iterator memory alocation\n");
        return NULL;
    }

    res->current = queue->end;
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
