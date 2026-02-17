#include "worker_model.h"

/* ================================ */
/*                                  */
/*      WorkerModel Constructot     */
/*                                  */
/* ================================ */
WorkerModel* create_worker(const char *first_name, const char *second_name) {
    WorkerModel *res;

    if ((res = (WorkerModel*) malloc(sizeof(WorkerModel))) == NULL) {
        fprintf(stderr, "Error while memory alocation for WorkerModel");
    }

    res->id = 0;

    res->roles = NULL;
    res->roles_count = 0;
    res->roles_included = 0;

    strcpy_s(res->first_name, first_name, WORKER_FIRST_NAME_MAX_SIZE);
    strcpy_s(res->second_name, second_name, WORKER_SECOND_NAME_MAX_SIZE);

    return res;
}
/* ================================ */
/*                                  */
/*        WorkerModel Setters       */
/*                                  */
/* ================================ */
void set_worker_first_name(WorkerModel *worker, const char *value) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    strcpy_s(worker->first_name, value, WORKER_FIRST_NAME_MAX_SIZE);
}

void set_worker_last_name(WorkerModel *worker, const char *value) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    strcpy_s(worker->second_name, value, WORKER_SECOND_NAME_MAX_SIZE);
}

/* ================================ */
/*                                  */
/*        WorkerModel Getters       */
/*                                  */
/* ================================ */
unsigned int get_worker_id(WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->id;
}

const char* get_worker_first_name(WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->first_name;
}

const char* get_worker_second_name(WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    return worker->second_name;
}

const RoleModel* get_worker_role(WorkerModel *worker, size_t index) {
    if (worker->roles == NULL || !worker->roles_included) {
        fprintf(stderr, "Role index out of bounds : worker roles are not included\n");
        exit(EXIT_FAILURE);

    }
    if (index >= worker->roles_count) {
        fprintf(stderr, "Role index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return &worker->roles[index];
}

size_t get_worker_roles_count(WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->roles_count;
}

/* ================================ */
/*                                  */
/*      WorkerModel Destructor      */
/*                                  */
/* ================================ */
void free_worker(WorkerModel* worker) {
    if (worker->roles != NULL)
        free(worker->roles);
    free(worker);
}

/* ================================ */
/*                                  */
/*        DATABASE FUNCTIONS        */
/*                                  */
/* ================================ */
WorkerModel* add_worker(MYSQL *conn, WorkerModel* worker) {
    MYSQL_BIND bind[2];
    MYSQL_STMT *stmt;

    char *query;
    
    if (worker->id != 0) {
        fprintf(stderr, "Worker is invalid or already exists with this id\n");
        return NULL;
    }

    memset(bind, 0, sizeof(bind));
    mysql_set_string_prop_bind(bind, worker->first_name);  // first_name
    mysql_set_string_prop_bind(bind, worker->second_name); // last_name

    query = "INSERT INTO worker(first_name, second_name) VALUES (?, ?)";

    if (mysql_request(conn, &stmt, query, NULL, bind)) {
        fprintf(stderr, "Error while worker inserting\n");
        return NULL;
    }
    worker->id = mysql_stmt_insert_id(stmt);
    
    mysql_stmt_close(stmt);
    return worker;
}

WorkerModel* select_worker_by_id(MYSQL *conn, unsigned int id) {
    WorkerModel *res;
    MYSQL_STMT *stmt;
    
    /* BINDS FOR REQUEST */
    MYSQL_BIND res_bind[2];
    MYSQL_BIND props_bind[1];

    char *query;

    char first_name[WORKER_FIRST_NAME_MAX_SIZE], 
         second_name[WORKER_SECOND_NAME_MAX_SIZE];
    unsigned long first_name_len, second_name_len;

    memset(res_bind, 0, sizeof(res_bind));
    memset(props_bind, 0, sizeof(props_bind));

    /*      RESULT BIND        */
    mysql_set_string_result_bind(res_bind + 0, first_name, sizeof(first_name), &first_name_len);    // first_name
    mysql_set_string_result_bind(res_bind + 1, second_name, sizeof(second_name), &second_name_len); // second_name

    /*     CONDITION BIND      */
    mysql_set_uint_prop_bind(props_bind, &id); // id

    query = "SELECT first_name, second_name FROM worker WHERE id = ?";

    if (mysql_request(conn, &stmt, query, res_bind, props_bind)) {
        fprintf(stderr, "Error while worker inserting\n");
        return NULL;
    }

    if (mysql_stmt_fetch(stmt) == 0) {

        first_name[first_name_len] = '\0';
        second_name[second_name_len] = '\0';

        if ((res = create_worker(first_name, second_name)) != NULL) {
            res->id = id;
        } else {
            res = NULL;
        }
    } else {
        res = NULL;
    }
    
    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return res;
}

Queue* select_workers(MYSQL *conn) {
    MYSQL_BIND bind[3];
    MYSQL_STMT *stmt;

    unsigned int id;
    char first_name[WORKER_FIRST_NAME_MAX_SIZE], second_name[WORKER_SECOND_NAME_MAX_SIZE];
    unsigned long first_name_len, second_name_len;

    WorkerModel *tmp;
    Queue *res;

    memset(bind, 0, sizeof(bind));

    mysql_set_uint_result_bind(bind, &id);                                                      // id
    mysql_set_string_result_bind(bind + 1, first_name, sizeof(first_name), &first_name_len);    // first_name
    mysql_set_string_result_bind(bind + 2, second_name, sizeof(second_name), &second_name_len); // second_name

    if (mysql_request(conn, &stmt, 
                      "SELECT id, first_name, second_name FROM worker", 
                      bind, NULL)) {
        return NULL;
    }

    if ((res = create_queue()) == NULL) {
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return NULL;
    }

    while (mysql_stmt_fetch(stmt) == 0) {

        first_name[first_name_len] = '\0';
        second_name[second_name_len] = '\0';

        if ((tmp = create_worker(first_name, second_name)) != NULL) {
            tmp->id = id;
            push_queue_element(res, tmp);
        }
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return res;
}

WorkerModel* include_worker_roles(MYSQL *conn, WorkerModel *worker) {
    MYSQL_STMT *stmt;
    MYSQL_BIND res_bind[2];
    MYSQL_BIND props_bind[1];
    char *query;

    int i = 0;
    unsigned int role_id;
    char role_name[ROLE_NAME_MAX_SIZE];
    unsigned long role_name_len;

    /* check on worker data */
    if (worker == NULL) {
        fprintf(stderr, "select_worker_roles : worker is invalide or NULL\n");
        return worker;
    }

    /* clearing worker roles array */
    if (worker->roles != NULL) {
        free(worker->roles);
        worker->roles = NULL;
        worker->roles_count = 0;
    }
    
    memset(res_bind, 0, sizeof(res_bind));
    memset(props_bind, 0, sizeof(props_bind));

    mysql_set_uint_result_bind(res_bind, &role_id);                                               // role_id
    mysql_set_string_result_bind(res_bind + 1, role_name, sizeof(role_name), &role_name_len);     // role_name

    mysql_set_uint_prop_bind(props_bind, &worker->id);                                            // id 


    query = "SELECT `role`.id, `role`.name "
            "FROM `role`, worker_role AS wr "
            "WHERE wr.role_id = `role`.id "
            "AND wr.worker_id = ?";

    if (mysql_request(conn, &stmt, query, res_bind, props_bind)) {
        return worker;
    }

    worker->roles_count = mysql_stmt_num_rows(stmt);
    if (worker->roles_count != 0 && 
        (worker->roles = (RoleModel*) malloc(sizeof(RoleModel) * worker->roles_count)) == NULL) {
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return worker;
    }

    while (mysql_stmt_fetch(stmt) == 0) {

        role_name[role_name_len] = '\0';
        
        worker->roles[i].id = role_id;
        strcpy(worker->roles[i].name, role_name);

        i++;
    }

    worker->roles_included = 1;

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return worker;
}
