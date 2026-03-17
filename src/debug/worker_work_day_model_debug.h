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

    printf("\n" HEADER_PADDING "START WorkerWorkDay DATABASE FUNCTIONS TEST:\n");

    /* -------- select_worker_work_day_by_id -------- */
    printf(TEXT_PADDING "select_worker_work_day_by_id: - ");

    wwd = select_worker_work_day_by_id(conn, 7);
    if (wwd == NULL) {
        printf("Error\n" TEXT_PADDING "Returned NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- include_worker -------- */
    printf(TEXT_PADDING "include_worker:               - ");

    if (include_worker(conn, wwd) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok\n");

    /* -------- include_line -------- */
    printf(TEXT_PADDING "include_line:                 - ");

    if (include_line(conn, wwd) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");
    
    /* -------- include_work_time_list -------- */
    printf(TEXT_PADDING "include_work_time_list:       - ");
    
    if (include_work_time_list(conn, wwd) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    if (get_work_time_list_count(get_worker_work_day_work_time(wwd)) == 0) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    
    free_worker_work_day(wwd);
}