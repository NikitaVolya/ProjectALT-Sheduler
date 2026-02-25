
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "database/worker_model.h"


int main() {
    MYSQL *conn;
    Queue *queue;
    QueueIterator *it;
    WorkerModel *worker; 

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    queue = select_workers(conn);
    printf("REQUEST DONE\n");

    for(it = get_queue_iterator(queue); 
        !queue_iterator_is_end(it);
        queue_iterator_forward(it)) {

        worker = get_queue_iterator_value(it);
        print_worker(worker);
    }

    close_queue_iterator(it);
    free_queue(queue);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
