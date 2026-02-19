#include <mariadb/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "database/worker_model.h"

int find_worker_by_id(void *value) {
    WorkerModel *worker = (WorkerModel *) value;

    return worker->id == 58;
}

int main() {
    MYSQL *conn;
    WorkerModel *worker = NULL;
    Queue *queue;
    QueueIterator *it;
    
    if (!(conn = mysql_init(0))) {
        fprintf(stderr, "unable to initialize connection struct\n");
        exit(1);
    }

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    printf("Connected succesfully!\n");

    queue = select_workers(conn);

    for (it = get_queue_iterator(queue); !queue_iterator_is_end(it); queue_iterator_forward(it)) {
        print_worker(get_queue_iterator_value(it));
    }
    printf("======================\n");

    worker = get_queue_element(queue, 1);
    set_worker_first_name(worker, "Hello");

    for (it = get_queue_iterator(queue); !queue_iterator_is_end(it); queue_iterator_forward(it)) {
        print_worker(get_queue_iterator_value(it));
    }
    printf("======================\n");

    update_workers(conn, queue);
    
    free_queue(queue);

    printf("\n======================\n");
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
