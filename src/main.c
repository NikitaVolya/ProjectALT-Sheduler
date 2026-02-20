#include <mariadb/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "database/worker_model.h"

int find_worker_by_id(void *value) {
    WorkerModel *worker = (WorkerModel *) value;

    return strstr(worker->first_name, "es") != NULL;
}

int main() {
    MYSQL *conn;
    Queue *queue;
    QueueIterator *it;
    RoleModel *role;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    printf("Connected succesfully!\n");

    queue = select_roles(conn);

    for (it = get_queue_iterator(queue); !queue_iterator_is_end(it); queue_iterator_forward(it)) {
        print_role(get_queue_iterator_value(it));
    }
    printf("======================\n");

    role = select_role_by_id(conn, 10);
    print_role(role);

    if (role != NULL)
        free_role(role);
    
    free_queue(queue);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
