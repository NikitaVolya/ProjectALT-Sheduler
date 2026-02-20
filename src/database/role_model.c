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

/* ================================ */
/*                                  */
/*        WorkerModel functions     */
/*                                  */
/* ================================ */
void fprint_role(FILE *file, const RoleModel *role) {
    if (role == NULL) {
        fprintf(file, "Role is NULL\n");
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

    if (mysql_request_f(conn, &stmt, res_bind, "SELECT name FROM `role` WHERE id = %ui ", &id)) {
        fprintf(stderr, "Error while worker selecting\n");
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

    set_queue_element_free_function(res, &free);

    return res;
}