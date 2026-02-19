#include <mariadb/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "database/worker_model.h"

int find_worker_by_id(void *value) {
    WorkerModel *worker = (WorkerModel *) value;

    return worker->id == 58;
}

void print_worker(WorkerModel *worker) {
    size_t i;

    printf("\n======================\n\n");

    if (worker == NULL) {
        printf("NULL\n");
    } else {
        printf("%d %s %s | %ld", 
            get_worker_id(worker), 
            get_worker_first_name(worker), 
            get_worker_second_name(worker), 
            get_worker_roles_count(worker));

        for (i = 0; i < get_worker_roles_count(worker); i++)
            printf(" %s ", get_role_name(get_worker_role(worker, i)));
        printf("\n");
    }
    
}

int main() {
    MYSQL *conn;
    WorkerModel *worker = NULL;
    
    if (!(conn = mysql_init(0))) {
        fprintf(stderr, "unable to initialize connection struct\n");
        exit(1);
    }

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    printf("Connected succesfully!\n");

    worker = create_worker("Nikita", "Volia");
    add_worker(conn, worker);

    print_worker(worker);

    free_worker(worker);

    printf("\n======================\n");
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
