#include <stdlib.h>
#include <stdio.h>

#include "debug_setup.h"
#include "../database/worker_model.h"
#include "../database/role_model.h"
#include "../utils.h"

void run_worker_tests(MYSQL *conn) {
    WorkerModel *worker, *tmp_worker;
    RoleModel *role;
    Queue *queue;
    unsigned int worker_id;

    if (conn == NULL) {
        printf("Test is impossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf(HEADER_PADDING "START WorkerModel FUNCTIONS TEST:\n");

    /* ================= CONSTRUCTOR ================= */

    printf(TEXT_PADDING "create_worker:       - ");

    worker = create_worker("John", "Smith");
    if (worker == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    /* ================= COPY ================= */

    printf(TEXT_PADDING "create_worker_copy:  - ");

    tmp_worker = create_worker_copy(worker);
    if (tmp_worker == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    else if (strcmp(worker->first_name, tmp_worker->first_name) != 0 ||
             strcmp(worker->second_name, tmp_worker->second_name) != 0 ||
             worker->id != tmp_worker->id) {
        printf("Error\nCopy data mismatch\n");
        exit(EXIT_FAILURE);
    }

    free_worker(tmp_worker);
    printf("OK\n");

    /* ================= DATABASE TESTS ================= */

    printf("\n" HEADER_PADDING "START WorkerModel DATABASE FUNCTIONS TEST:\n");

    /* -------- add_worker -------- */

    printf(TEXT_PADDING "add_worker:          - ");
    if (add_worker(conn, worker) == NULL) {
        printf("Error\nError while inserting\n");
        exit(EXIT_FAILURE);
    }
    else if (worker->id == 0) {
        printf("Error\nWrong ID after insert\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    /* -------- select_worker_by_id -------- */

    printf(TEXT_PADDING "select_worker_by_id: - ");
    worker_id = get_worker_id(worker);

    tmp_worker = select_worker_by_id(conn, worker_id);
    if (tmp_worker == NULL) {
        printf("Error\nReturned NULL\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }
    else if (strcmp(worker->first_name, tmp_worker->first_name) != 0 ||
             strcmp(worker->second_name, tmp_worker->second_name) != 0) {
        printf("Error\nData mismatch\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- refresh_worker --------*/

    printf(TEXT_PADDING "refresh_worker:      - ");
    set_worker_first_name(tmp_worker, "Hello, world");
    if (refresh_worker(conn, &tmp_worker) == NULL) {
        printf(TEXT_PADDING "Error\nReturn NULL\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }
    else if (strcmp(worker->first_name, tmp_worker->first_name) != 0) {
        printf(TEXT_PADDING "Error\nData is different\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    /* -------- update_worker -------- */

    printf(TEXT_PADDING "update_worker:       - ");
    set_worker_first_name(worker, "Michael");

    if (update_worker(conn, worker) == NULL) {
        printf("Error\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }

    refresh_worker(conn, &tmp_worker);

    if (strcmp(worker->first_name, tmp_worker->first_name) != 0) {
        printf("Error\nDatabase not updated\n");
        delete_worker(conn, worker);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- add_worker_role -------- */

    printf(TEXT_PADDING "add_worker_role:     - ");

    role = create_role("Developer");
    add_role(conn, role);
    
    include_worker_roles(conn, worker);

    if (add_worker_role(conn, &worker, role) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    if (get_worker_roles_count(worker) == 0) {
        printf("Error\nRole not added\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    
    /* -------- select_workers_by_role -------- */

    printf(TEXT_PADDING "select_workers_by_role: - ");

    queue = select_workers_by_role(conn, role);

    if (queue == NULL) {
        fprintf(stderr, "Error\n");
        exit(EXIT_FAILURE);
    }

    if (get_queue_size(queue) != 1) {
        free_queue(queue);
        fprintf(stderr, "Error, queue size is unexpected\n");
        exit(EXIT_FAILURE);
    }
    
    printf("OK\n");

    free_queue(queue);

    /* -------- remove_worker_role -------- */
    
    printf(TEXT_PADDING "remove_worker_role:  - ");

    if (remove_worker_role(conn, &worker, role) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    if (get_worker_roles_count(worker) != 0) {
        printf("Error\nRole not removed\n");
        exit(EXIT_FAILURE);
    }
    
    printf("OK\n");

    /* -------- delete_worker -------- */

    printf(TEXT_PADDING "delete_worker:       - ");

    if (delete_worker(conn, worker) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    else if (worker->id != 0) {
        printf("Error\nID not reset\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    free_worker(tmp_worker);
    free_worker(worker);
    delete_role(conn, role);
    free_role(role);

    /* -------- select_workers -------- */

    printf(TEXT_PADDING "select_workers:      - ");

    queue = select_workers(conn);
    if (queue == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    free_queue(queue);
    
    /* ================= WorkerModel SUMMARY ================= */

    printf("\n" HEADER_PADDING "WorkerModel SUMMARY:    - OK\n");
}