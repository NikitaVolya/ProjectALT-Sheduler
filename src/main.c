#include <mariadb/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "database/worker_model.h"


int main() {
    MYSQL *conn;
    Queue *queue;
    QueueIterator *it;

    WorkerModel *worker;
    RoleModel *role;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    printf("Connected succesfully!\n");

    role = select_role_by_id(conn, 4);
    worker = select_worker_by_id(conn, 1);

    include_worker_roles(conn, worker);

    print_worker(worker);
    print_role(role);

    add_worker_role(conn, &worker, role);

    print_worker(worker);
    
    printf("======================\n");

    queue = select_roles(conn);

    for (it = get_queue_iterator(queue); !queue_iterator_is_end(it); queue_iterator_forward(it)) {
        print_role(get_queue_iterator_value(it));
    }
    printf("======================\n");
    
    free_worker(worker);
    free_role(role);
    free_queue(queue);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
