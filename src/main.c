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
    size_t i;
    
    if (!(conn = mysql_init(0))) {
        fprintf(stderr, "unable to initialize connection struct\n");
        exit(1);
    }

    if (!mysql_real_connect(
        conn,
        "127.0.0.1",
        "app_user",
        "strong_password",
        "test",
        3306,
        NULL,
        0
    )) {
        printf("Connection failed: %s\n", mysql_error(conn));
        exit(1);
    }
    
    printf("Connected succesfully!\n");
    
    printf("\n======================\n\n");

    worker = select_worker_by_id(conn, 1);
    include_worker_roles(conn, worker);

    if (worker == NULL) {
        printf("NULL\n");
    } else {
        printf("%d %s %s | %ld", 
            get_worker_id(worker), 
            get_worker_first_name(worker), 
            get_worker_second_name(worker), 
            get_worker_roles_count(worker));
    }

    for (i = 0; i < get_worker_roles_count(worker); i++)
        printf(" %s ", get_worker_role(worker, i)->name);
    printf("\n");

    free_worker(worker);

    
    printf("\n======================\n");
    
    mysql_close(conn);

    
    exit(EXIT_SUCCESS);
}
