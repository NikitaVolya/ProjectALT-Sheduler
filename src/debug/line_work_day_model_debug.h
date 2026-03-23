#include <stdlib.h>
#include <stdio.h>

#include "debug_setup.h"
#include "../database/line_work_day_model.h"
#include "../utils.h"

void run_line_work_day_tests(MYSQL *conn) {
    LineWorkDayModel *lwd;
    MYSQL_TIME date, end;


    if (conn == NULL) {
        printf("Test is impossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("\n" HEADER_PADDING "START LineWorkDay DATABASE FUNCTIONS TEST:\n");

    /* -------- select_line_work_day_by_id -------- */
    printf(TEXT_PADDING "select_line_work_day_by_id: - ");

    lwd = select_line_work_day_by_id(conn, 6);
    if (lwd == NULL) {
        printf("Error\n" TEXT_PADDING "Returned NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");


    /* -------- include_line_work_day_line -------- */
    printf(TEXT_PADDING "include_line_work_day_line: - ");

    if (include_line_work_day_line(conn, lwd) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");
    
    /* -------- include_line_work_day_work_time_list -------- */
    printf(TEXT_PADDING "include_line_work_day_work_time_list: - ");
    
    if (include_line_work_day_work_time_list(conn, lwd) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    if (get_work_time_list_count(get_line_work_day_work_time(lwd)) == 0) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");
    
    free_line_work_day(lwd);

    /* -------- create_line_work_day_model -------- */
    printf(TEXT_PADDING "create_line_work_day_model: - ");

    memset(&date, 0, sizeof(date));
    date.year = 2026;
    date.month = 2;
    date.day = 10;

    if ((lwd = create_line_work_day_model(conn, date, 1)) == NULL) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");
    

    /* -------- add_line_work_day_work_time -------- */
    printf(TEXT_PADDING "add_line_work_day_work_time:- ");
    
    memset(&date, 0, sizeof(date));
    memset(&end, 0, sizeof(end));

    date.hour = 10;
    date.minute = 30;
    date.second = 0;

    end.hour = 15;
    end.minute = 0;
    end.second = 0;

    if (add_line_work_day_work_time(conn, lwd, date, end) == NULL ||
        get_work_time_list_count(get_line_work_day_work_time(lwd)) != 1) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    
    /* -------- remove_line_work_day_work_time -------- */
    printf(TEXT_PADDING "remove_line_work_day_work_time:- ");

    date.hour = 12;
    date.minute = 30;
    date.second = 0;

    end.hour = 13;
    end.minute = 0;
    end.second = 0;

    if (remove_line_work_day_work_time(conn, lwd, date, end) == NULL ||
        get_work_time_list_count(get_line_work_day_work_time(lwd)) != 2) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }
    
    printf("OK\n");


    /* -------- delete_line_work_day -------- */
    printf(TEXT_PADDING "delete_line_work_day:       - ");

    if (delete_line_work_day(conn, lwd)) {
        printf("Error\n" TEXT_PADDING "unexpected result\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");
}