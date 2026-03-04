#include <stdlib.h>
#include <stdio.h>

#include "debug_setup.h"
#include "../database/line_model.h"
#include "../utils.h"

void run_line_tests(MYSQL *conn) {
    LineModel *line, *tmp_line;
    RoleModel *role;
    Queue *queue;
    unsigned int line_id;

    if (conn == NULL) {
        printf("Test is impossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf(HEADER_PADDING "START LineModel FUNCTIONS TEST:\n");

    /* ================= CONSTRUCTOR ================= */

    printf(TEXT_PADDING "create_line:         - ");

    line = create_line("Line A", "Product A");
    if (line == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* ================= COPY ================= */

    printf(TEXT_PADDING "create_line_copy:    - ");

    tmp_line = create_line_copy(line);
    if (tmp_line == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    else if (strcmp(line->name, tmp_line->name) != 0 ||
             strcmp(line->product_name, tmp_line->product_name) != 0 ||
             line->id != tmp_line->id) {
        printf("Error\nCopy data mismatch\n");
        exit(EXIT_FAILURE);
    }

    free_line(tmp_line);
    printf("OK\n");

    printf("\n" HEADER_PADDING "START LineModel DATABASE FUNCTIONS TEST:\n");

    /* -------- add_line -------- */

    printf(TEXT_PADDING "add_line:            - ");

    if (add_line(conn, line) == NULL) {
        printf("Error\nInsert failed\n");
        exit(EXIT_FAILURE);
    }
    else if (line->id == 0) {
        printf("Error\nWrong ID after insert\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- select_line_by_id -------- */

    printf(TEXT_PADDING "select_line_by_id:   - ");

    line_id = line->id;
    tmp_line = select_line_by_id(conn, line_id);

    if (tmp_line == NULL) {
        printf("Error\nReturned NULL\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }
    else if (strcmp(line->name, tmp_line->name) != 0 ||
             strcmp(line->product_name, tmp_line->product_name) != 0) {
        printf("Error\nData mismatch\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- refresh_line -------- */

    printf(TEXT_PADDING "refresh_line:        - ");

    set_line_name(tmp_line, "Changed Name");

    if (refresh_line(conn, &tmp_line) == NULL) {
        printf("Error\nReturn NULL\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }
    else if (strcmp(line->name, tmp_line->name) != 0) {
        printf("Error\nData mismatch after refresh\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- update_line -------- */

    printf(TEXT_PADDING "update_line:         - ");

    set_line_name(line, "Updated Line");

    if (update_line(conn, line) == NULL) {
        printf("Error\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }

    refresh_line(conn, &tmp_line);

    if (strcmp(line->name, tmp_line->name) != 0) {
        printf("Error\nDatabase not updated\n");
        delete_line(conn, line);
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- include_line_roles -------- */

    printf(TEXT_PADDING "include_line_roles:  - ");

    role = create_role("Operator");
    add_role(conn, role);

    if (include_line_roles(conn, line) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    /* -------- delete_line -------- */

    printf(TEXT_PADDING "delete_line:         - ");

    if (delete_line(conn, line) == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }
    else if (line->id != 0) {
        printf("Error\nID not reset\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    
    free_line(tmp_line);
    free_line(line);

    /* -------- select_lines -------- */

    printf(TEXT_PADDING "select_lines:        - ");

    queue = select_lines(conn);
    if (queue == NULL) {
        printf("Error\n");
        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    free_queue(queue);

    delete_role(conn, role);
    free_role(role);

    printf("\n" HEADER_PADDING "LineModel SUMMARY:      - OK\n");
}