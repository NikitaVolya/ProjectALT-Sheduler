#include <stdlib.h>
#include <stdio.h>

#include "debug_setup.h"
#include "../database/role_model.h"
#include "../utils.h"


void run_role_tests(MYSQL *conn) {
    RoleModel *role, *tmp_role;
    Queue *queue;
    unsigned int role_id;

    if (conn == NULL) {
        printf("Test is inpossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }
    printf(HEADER_PADDING "START RoleModel FUNCTIONS TEST:\n");

    printf(TEXT_PADDING "create_role:       - ");

    role = create_role("test role");
    if (role == NULL) {
        printf(TEXT_PADDING "Error\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    printf(TEXT_PADDING "create_role_copy:  - ");

    tmp_role = create_role_copy(role);
    if (tmp_role == NULL) {
        printf(TEXT_PADDING "Error\ncreate_role_copy return NULL\n");
        exit(EXIT_FAILURE);
    }
    else if (strcmp(role->name, tmp_role->name) != 0) {
        printf(TEXT_PADDING "Error\ncopy name is different\n");
        exit(EXIT_FAILURE);
    }
    else if (role->id != tmp_role->id) {
        printf(TEXT_PADDING "Error\ncopy id is different\n");
        exit(EXIT_FAILURE);
    } 
    free_role(tmp_role);    
    printf("OK\n");
    
    printf("\n" HEADER_PADDING "START RoleModel DATABASE FUNCTIONS TEST:\n");
    
    printf(TEXT_PADDING "add_role:          - ");
    add_role(conn, role);
    if (role == NULL) {
        printf(TEXT_PADDING "Error\nRoleModel is not added to database\n");
        delete_role(conn, role);
        exit(EXIT_FAILURE);
    }
    if (role->id == 0) {
        delete_role(conn, role);
        printf(TEXT_PADDING "Error\nRoleModel have wrong id in return\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");
    
    printf(TEXT_PADDING "select_role_by_id: - ");
    role_id = get_role_id(role);

    tmp_role = select_role_by_id(conn, role_id);
    if (role == NULL) {
        printf("Error\n");
        
        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    else if (strcmp(role->name, tmp_role->name) != 0) {
        printf(TEXT_PADDING "Error\nReturned data is wrong\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }

    printf("OK\n");

    printf(TEXT_PADDING "refresh_role:      - ");
    set_role_name(tmp_role, "Hello, world");
    if (refresh_role(conn, &tmp_role) == NULL) {
        printf(TEXT_PADDING "Error\nReturn NULL\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    else if (strcmp(role->name, tmp_role->name) != 0) {
        printf(TEXT_PADDING "Error\nData is different\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    printf(TEXT_PADDING "update_role:       - ");

    set_role_name(role, "Orange");
    if (update_role(conn, role) == NULL) {
        printf(TEXT_PADDING "Error\nReturn NULL\n");

        delete_role(conn, tmp_role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    refresh_role(conn, &tmp_role);
    if (strcmp(role->name, tmp_role->name) != 0) {
        printf(TEXT_PADDING "Error\nData in database in not updated\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    printf(TEXT_PADDING "delete_role:       - ");
    if (delete_role(conn, role) == NULL) {
        printf(TEXT_PADDING "Error\nInvalide RoleModel\n");
    } else if (role->id != 0) {
        printf(TEXT_PADDING "Error\nReturn RoleModel have an id\n");
    } else {
        printf("OK\n");
    }

    free_role(tmp_role);
    free_role(role);
    printf("\n" HEADER_PADDING "START RoleModel QUEUE DATABASE FUNCTIONS TEST:\n");
    printf(TEXT_PADDING "select_roles:      - ");

    queue = select_roles(conn);
    if (queue == NULL) {
    printf(TEXT_PADDING "select_roles:      - ");
        printf(TEXT_PADDING "Error\nResult is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (get_queue_size(queue) == 0) {
        printf(TEXT_PADDING "Error\nQueue is empty\n");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    free_queue(queue);
    printf("\n" HEADER_PADDING "RoleModel SUMMARY:    - OK\n");
}