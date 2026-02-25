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
    REQUESTF_RESULT *request_result;
    
    if (role == NULL || role->id != 0) {
        fprintf(stderr, "Error : RoleModel is NULL or already exists\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn, "INSERT INTO `role`(name) VALUES (%s)", role->name);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while RoleModel inserting\n");
        free_requestf_result(request_result);
        return NULL;
    }
    role->id = get_requestf_insert_id(request_result);
    role->is_changed = 0;
    
    free_requestf_result(request_result);
    return role;
}

RoleModel* select_role_by_id(MYSQL *conn, unsigned int id) {
    REQUESTF_RESULT *request_result;
    RoleModel *res;

    if ((res = create_role("")) == NULL) {
        fprintf(stderr, "Error while memory alocation of RoleModel\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn, "SELECT name FROM `role` WHERE id = %ui", &id,
        MYSQL_BIND_STRING);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while worker selecting\n");
        free_role(res);
        free_requestf_result(request_result);
        return NULL;
    }

    if (requestf_result_fetch(request_result, ROLE_NAME_MAX_SIZE, res->name) == 0) {
        res->id = id;
    } else {
        free_role(res);
        res = NULL;
    }
    
    free_requestf_result(request_result);
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
    REQUESTF_RESULT *request_result;

    if (role == NULL || role->id == 0) {
        fprintf(stderr, "Error : RoleModel is NULL or not exist in data base\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn, 
        "UPDATE `role` SET name = %s WHERE id = %ui", role->name, &role->id);

    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return role;
    }

    if (get_requestf_affected_rows(request_result) == 0 && role->is_changed) {
        fprintf(stderr, "RoleModel is not updated\n");
        role = NULL;
    } else {
        role->is_changed = 0;
    }

    free_requestf_result(request_result);
    return role;
}


RoleModel* delete_role(MYSQL *conn, RoleModel *role) {
    REQUESTF_RESULT *request_result;

    if (role == NULL || role->id == 0) {
        fprintf(stderr, "Error : RoleModel is NULL or not exist in data base\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn, "DELETE FROM `role` WHERE id = %ui ", &role->id);
    
    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return role;
    }

    if (get_requestf_affected_rows(request_result) == 0) {
        fprintf(stderr, "RoleModel is not deleted\n");
    }

    role->id = 0;
    role->is_changed = 0;

    free_requestf_result(request_result);
    return role;
}

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_roles(MYSQL *conn) {
    REQUESTF_RESULT *request_result;
    RoleModel *new_element, tmp;
    Queue *res;

    request_result = mysql_request_f_result(conn, "SELECT id, name FROM `role`",
        MYSQL_BIND_UINT, MYSQL_BIND_STRING);

    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return NULL;
    }

    if ((res = create_queue()) == NULL) {
        free_requestf_result(request_result);
        return NULL;
    }

    while (requestf_result_fetch(request_result,
        &tmp.id, 
        ROLE_NAME_MAX_SIZE, tmp.name) == 0) {

        if ((new_element = create_role_copy(&tmp)) != NULL) {
            push_queue_element(res, new_element);
        }
    }

    set_queue_element_free_function(res, &free_role);
    free_requestf_result(request_result);

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
