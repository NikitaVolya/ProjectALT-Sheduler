#include <stdlib.h>
#include <stdio.h>

#include "debug_setup.h"
#include "../database/worker_work_day_model.h"
#include "../utils.h"

void run_worker_work_day_tests(MYSQL *conn) {
    WorkerWorkDayModel *wwd;

    if (conn == NULL) {
        printf("Test is impossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("\n" HEADER_PADDING "START WorkerModel DATABASE FUNCTIONS TEST:\n");

    /* -------- select_worker_work_day_by_id -------- */
    printf(TEXT_PADDING "select_worker_work_day_by_id: - ");

    wwd = select_worker_work_day_by_id(conn, 7);
    if (wwd == NULL) {
        printf("Error\nReturned NULL\n");
        free_worker_work_day(wwd);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- include_worker --------*/
    printf(TEXT_PADDING "include_worker: - ");

    if (include_worker(conn, wwd) == NULL) {
        printf("Error\n");
    }
    printf("Ok\n");

    /* -------- include_line --------*/
    printf(TEXT_PADDING "include_line: - ");

    if (include_line(conn, wwd) == NULL) {
        printf("Error\n");
    }
    printf("Ok\n");

    printf("\n");
    include_worker_roles(conn, wwd->worker);
    include_line_roles(conn, wwd->line);
    print_worker_work_day(wwd);
    
    free_worker_work_day(wwd);
}