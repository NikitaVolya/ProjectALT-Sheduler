#include"role_model.h"

/* ================================ */
/*                                  */
/*       RoleModel Constructot      */
/*                                  */
/* ================================ */
RoleModel* create_role(const char *name) {
    RoleModel *res;

    if ((res = (RoleModel*) malloc(sizeof(RoleModel))) == NULL) {
        fprintf(stderr, "Error while memory alocation for RoleModel");
        return NULL;
    }

    res->id = 0;
    res->is_changed = 0;

    strcpy_s(res->name, name, ROLE_NAME_MAX_SIZE);

    return res;
}

void* create_role_copy(void *value) {
    RoleModel *role, *res;

    role = value;

    if (role == NULL) {
        fprintf(stderr, "Error RoleModel is NULL\n");
        return NULL;
    }

    if ((res = create_role(role->name)) == NULL) {
        return NULL;
    }

    res->id = role->id;
    res->is_changed = role->is_changed;

    return res;
}


/* ================================ */
/*                                  */
/*        WorkerModel Setters       */
/*                                  */
/* ================================ */
void set_role_name(RoleModel *role, const char *value) {
    if (role == NULL) {
        fprintf(stderr, "Error : role is NULL\n");
        exit(EXIT_FAILURE);
    }

    strcpy_s(role->name, value, ROLE_NAME_MAX_SIZE);

    role->is_changed = 1;
}

/* ================================ */
/*                                  */
/*        WorkerModel Getters       */
/*                                  */
/* ================================ */
const char* get_role_name(const RoleModel *role) {
    if (role == NULL) {
        fprintf(stderr, "Error : role is NULL\n");
        exit(EXIT_FAILURE);
    }

    return role->name;
}

unsigned int get_role_id(const RoleModel *role) {
    if (role == NULL) {
        fprintf(stderr, "Error : role is NULL\n");
        exit(EXIT_FAILURE);
    }

    return role->id;
}

/* ================================ */
/*                                  */
/*        WorkerModel functions     */
/*                                  */
/* ================================ */
void fprint_role(FILE *file, const RoleModel *role) {
    if (role == NULL) {
        fprintf(file, "RoleModel is NULL\n");
    } else {
        fprintf(file, "< RoleModel %d : %s >\n", role->id, role->name);
    }
}

void print_role(const RoleModel *role) {
    fprint_role(stdout, role);
}

/* ================================ */
/*                                  */
/*       RoleModel destructor       */
/*                                  */
/* ================================ */
void free_role(void *value) {
    free(value);
}

/* ================================ */
/*                                  */
/*        DATABASE FUNCTIONS        */
/*                                  */
/* ================================ */
RoleModel* add_role(MYSQL *conn, RoleModel* role) {
    MYSQL_STMT *stmt;
    
    if (role == NULL || role->id != 0) {
        fprintf(stderr, "Error : RoleModel is NULL or already exists\n");
        return NULL;
    }

    if (mysql_request_f(conn, &stmt, NULL, "INSERT INTO `role`(name) VALUES (%s)", role->name)) {
        fprintf(stderr, "Error while RoleModel inserting\n");
        return NULL;
    }
    role->id = mysql_stmt_insert_id(stmt);
    role->is_changed = 0;
    
    mysql_stmt_close(stmt);
    return role;
}

RoleModel* select_role_by_id(MYSQL *conn, unsigned int id) {
    RoleModel *res;
    MYSQL_STMT *stmt;
    
    /* BINDS FOR REQUEST */
    MYSQL_BIND res_bind[1];
    unsigned long name_len;

    if ((res = create_role("")) == NULL) {
        fprintf(stderr, "Error while memory alocation of RoleModel\n");
        return NULL;
    }

    memset(res_bind, 0, sizeof(res_bind));

    /*      RESULT BIND        */
    mysql_set_string_result_bind(res_bind, res->name, sizeof(res->name), &name_len);    // name

    if (mysql_request_f(conn, &stmt, res_bind, "SELECT name FROM `role` WHERE id = %ui", &id)) {
        fprintf(stderr, "Error while worker selecting\n");
        free_role(res);
        return NULL;
    }

    if (mysql_stmt_fetch(stmt) == 0) {
        res->name[name_len] = '\0';
        res->id = id;
    } else {
        free_role(res);
        res = NULL;
    }
    
    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return res;
}

RoleModel* refresh_role(MYSQL *conn, RoleModel **role) {
    RoleModel *res;

    if (role == NULL || *role == NULL) {
        fprintf(stderr, "Error : RoleModel is NULL\n");
        return NULL;
    }

    if ((*role)->id == 0 || 
        (res = select_role_by_id(conn, (*role)->id )) == NULL) {
        fprintf(stderr, "Error : RoleModel is not found in data base\n");
        return NULL;
    }

    free_role(*role);
    *role = res;
    return res;
}

RoleModel* update_role(MYSQL *conn, RoleModel *role) {
    MYSQL_STMT *stmt;

    if (role == NULL || role->id == 0) {
        fprintf(stderr, "Error : RoleModel is NULL or not exist in data base\n");
        return NULL;
    }

    if (mysql_request_f(conn, &stmt, NULL, 
            "UPDATE `role` SET name = %s WHERE id = %ui", role->name, &role->id)) {
        return role;
    }

    if (mysql_stmt_affected_rows(stmt) == 0 && role->is_changed) {
        fprintf(stderr, "RoleModel is not updated\n");
        role = NULL;
    } else {
        role->is_changed = 0;
    }

    mysql_stmt_close(stmt);
    return role;
}


RoleModel* delete_role(MYSQL *conn, RoleModel *role) {
    MYSQL_STMT *stmt;

    if (role == NULL || role->id == 0) {
        fprintf(stderr, "Error : RoleModel is NULL or not exist in data base\n");
        return NULL;
    }
    
    if (mysql_request_f(conn, &stmt, NULL, 
        "DELETE FROM `role` WHERE id = %ui ", &role->id)) {
        return role;
    }

    if (mysql_stmt_affected_rows(stmt) == 0) {
        fprintf(stderr, "RoleModel is not deleted\n");
    }

    role->id = 0;
    role->is_changed = 0;

    mysql_stmt_close(stmt);
    return role;
}

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_roles(MYSQL *conn) {
    MYSQL_BIND bind[2];
    MYSQL_STMT *stmt;

    unsigned int id;
    char name[ROLE_NAME_MAX_SIZE];
    unsigned long name_len;

    RoleModel *tmp;
    Queue *res;

    memset(bind, 0, sizeof(bind));

    mysql_set_uint_result_bind(bind, &id);                                    // id
    mysql_set_string_result_bind(bind + 1, name, sizeof(name), &name_len);    // name

    if (mysql_request_f(conn, &stmt, bind, "SELECT id, name FROM `role`")) {
        return NULL;
    }

    if ((res = create_queue()) == NULL) {
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return NULL;
    }

    while (mysql_stmt_fetch(stmt) == 0) {

        name[name_len] = '\0';

        if ((tmp = create_role(name)) != NULL) {
            tmp->id = id;
            push_queue_element(res, tmp);
        }
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    set_queue_element_free_function(res, &free_role);

    return res;
}

Queue* refresh_roles(MYSQL *conn, Queue *roles) {
    RoleModel *role;
    size_t i = get_queue_size(roles);

    while (i > 0) {
        i--;

        role = pop_queue_element(roles);

        if (refresh_role(conn, &role) == NULL) {
            free_role(role);
        } else {
            push_queue_element(roles, role);
        }

    }
    return roles;
}

Queue* update_roles(MYSQL *conn, Queue *roles) {
    RoleModel *role;
    size_t i = get_queue_size(roles);

    while (i > 0) {
        i--;

        role = pop_queue_element(roles);

        update_role(conn, role);

        push_queue_element(roles, role);
    }
    return roles;
}

void delete_roles(MYSQL *conn, Queue *roles) {
    RoleModel *role;

    while ((role = pop_queue_element(roles)) != NULL) {
        delete_role(conn, role);
        free_role(role);
    }
}


/*=================================*/
/*          ROLE TESTS             */
/*=================================*/
void run_role_tests(MYSQL *conn) {
    RoleModel *role, *tmp_role;
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

    
    printf("===============================================\n");
    printf("RoleModel:         Ok\n");
    printf("===============================================\n");

    free_role(role);
    free_role(tmp_role);
}