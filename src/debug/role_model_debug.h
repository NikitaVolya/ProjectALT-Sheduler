#include <stdlib.h>
#include <stdio.h>

#include "../database/role_model.h"
#include "../utils.h"


void run_role_tests(MYSQL *conn) {
    RoleModel *role, *tmp_role;
    Queue *queue;
    unsigned int role_id;

    if (conn == NULL) {
        fprintf(stderr, "Test is inpossible because conn is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("===============================================\n");
    printf("Start RoleModel functions test\n");

    /* CREATE ROLE TEST*/
    printf("create_role:       ");

    role = create_role("test role");
    if (role == NULL) {
        fprintf(stderr, "Error\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok\n");

    /* CREATE ROLE COPY TEST */
    printf("create_role_copy:  ");

    tmp_role = create_role_copy(role);
    if (tmp_role == NULL) {
        fprintf(stderr, "Error\ncreate_role_copy return NULL\n");
        exit(EXIT_FAILURE);
    }
    else if (strcmp(role->name, tmp_role->name) != 0) {
        fprintf(stderr, "Error\ncopy name is different\n");
        exit(EXIT_FAILURE);
    }
    else if (role->id != tmp_role->id) {
        fprintf(stderr, "Error\ncopy id is different\n");
        exit(EXIT_FAILURE);
    } 
    free_role(tmp_role);    
    printf("Ok\n");

    
    printf("===============================================\n");
    printf("Start RoleModel database functions test\n");
    
    /* ADD ROLE FUNCTION TEST */
    printf("add_role:         ");
    add_role(conn, role);
    if (role == NULL) {
        fprintf(stderr, "Error\nRoleModel is not added to database\n");
        delete_role(conn, role);
        exit(EXIT_FAILURE);
    }
    if (role->id == 0) {
        delete_role(conn, role);
        fprintf(stderr, "Error\nRoleModel have wrong id in return\n");
        exit(EXIT_FAILURE);
    }
    printf(" Ok\n");
    
    /* SELECT ROLE BY ID TEST */
    printf("select_role_by_id: ");
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
        fprintf(stderr, "Error\nReturned data is wrong\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }

    printf("Ok\n");

    /* REFRESH ROLE TEST */
    printf("refresh_role:      ");
    set_role_name(tmp_role, "Hello, world");
    if (refresh_role(conn, &tmp_role) == NULL) {
        fprintf(stderr, "Error\nReturn NULL\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    else if (strcmp(role->name, tmp_role->name) != 0) {
        fprintf(stderr, "Error\nData is different\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    printf("Ok\n");

    /* UPDATE ROLE TEST */
    printf("update_role:       ");

    set_role_name(role, "Orange");
    if (update_role(conn, role) == NULL) {
        fprintf(stderr, "Error\nReturn NULL\n");

        delete_role(conn, tmp_role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    refresh_role(conn, &tmp_role);
    if (strcmp(role->name, tmp_role->name) != 0) {
        fprintf(stderr, "Error\nData in database in not updated\n");

        delete_role(conn, role);
        free_role(tmp_role);
        free_role(role);

        exit(EXIT_FAILURE);
    }
    printf("Ok\n");

    printf("delete_role:       ");
    if (delete_role(conn, role) == NULL) {
        fprintf(stderr, "Error\nInvalide RoleModel\n");
    } else if (role->id != 0) {
        fprintf(stderr, "Error\nReturn RoleModel have an id\n");
    } else {
        printf("Ok\n");
    }

    free_role(role);
    free_role(tmp_role);
    
    printf("===============================================\n");
    printf("Start RoleModel queue database functions test\n");
    printf("select_roles:      ");

    queue = select_roles(conn);
    if (queue == NULL) {
        fprintf(stderr, "Error\nResult is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (get_queue_size(queue) == 0) {
        fprintf(stderr, "Error\nQueue is empty\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok\n");

    free_queue(queue);

    printf("===============================================\n");
    printf("RoleModel:         Ok\n");
    printf("===============================================\n");

}