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
        return NULL;
    }

    res->id = 0;
    strcpy_s(res->first_name, first_name, WORKER_FIRST_NAME_MAX_SIZE);
    strcpy_s(res->second_name, second_name, WORKER_SECOND_NAME_MAX_SIZE);

    res->is_changed = 0;

    res->roles = NULL;
    res->roles_count = 0;
    res->roles_included = 0;

    return res;
}

void* create_worker_copy(void *value) {
    WorkerModel *res, *worker;
    size_t i;

    worker = (WorkerModel*) value;

    if ((res = create_worker(worker->first_name, worker->second_name)) == NULL) 
        return NULL;
    
    res->id = worker->id;

    res->is_changed = worker->is_changed;

    if (worker->roles_included) {
        res->roles_count = worker->roles_count;

        if ((res->roles = (RoleModel*) malloc(sizeof(RoleModel) * res->roles_count)) == NULL) {
            fprintf(stderr, "Error while memory alocation for WorkerModel roles\n");
            free_worker(res);
            return NULL;
        }

        for (i = 0; i < res->roles_count; i++) {
            res->roles[i] = worker->roles[i];
        }
        
        res->roles_included = 1;
    }

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

    worker->is_changed = 1;
}

void set_worker_second_name(WorkerModel *worker, const char *value) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    strcpy_s(worker->second_name, value, WORKER_SECOND_NAME_MAX_SIZE);

    worker->is_changed = 1;
}

/* ================================ */
/*                                  */
/*        WorkerModel Getters       */
/*                                  */
/* ================================ */
unsigned int get_worker_id(const WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->id;
}

const char* get_worker_first_name(const WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->first_name;
}

const char* get_worker_second_name(const WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    return worker->second_name;
}

const RoleModel* get_worker_role(const WorkerModel *worker, size_t index) {
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

size_t get_worker_roles_count(const WorkerModel *worker) {
    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        exit(EXIT_FAILURE);
    }

    return worker->roles_count;
}

/* ================================ */
/*                                  */
/*       WorkerModel functions      */
/*                                  */
/* ================================ */
void fprint_worker(FILE *file, const WorkerModel *worker) {
    size_t i;
    if (worker == NULL) {
        printf("WorkerModel is NULL\n");
    } else {
        fprintf(file, "< WorkerModel %d : %s %s [ ", 
            get_worker_id(worker), 
            get_worker_first_name(worker), 
            get_worker_second_name(worker));

        for (i = 0; i < get_worker_roles_count(worker); i++) {
            if (i != 0)
                fprintf(file, ", ");
            fprintf(file, "%s", get_role_name(get_worker_role(worker, i)));
        }
        fprintf(file, " ] >\n");
    }
    
}

void print_worker(const WorkerModel *worker) {
    fprint_worker(stdout, worker);
}

/* ================================ */
/*                                  */
/*      WorkerModel Destructor      */
/*                                  */
/* ================================ */
void free_worker(void *value) {
    WorkerModel *worker = (WorkerModel *) value;

    if (worker == NULL)
        return;

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
    MYSQL_STMT *stmt;
    
    if (worker == NULL || worker->id != 0) {
        fprintf(stderr, "Error : Worker is NULL or already exists\n");
        return NULL;
    }

    if (mysql_request_f(conn, &stmt, NULL, 
        "INSERT INTO worker(first_name, second_name) VALUES (%s, %s)", 
        worker->first_name, worker->second_name)) {
        fprintf(stderr, "Error while worker inserting\n");
        return NULL;
    }
    worker->id = mysql_stmt_insert_id(stmt);
    worker->is_changed = 0;
    
    mysql_stmt_close(stmt);
    return worker;
}


WorkerModel* select_worker_by_id(MYSQL *conn, unsigned int id) {
    WorkerModel *res;
    MYSQL_STMT *stmt;
    
    /* BINDS FOR REQUEST */
    MYSQL_BIND res_bind[2];
    unsigned long first_name_len, second_name_len;

    if ((res = create_worker("", "")) == NULL) {
        fprintf(stderr, "Error while memory alocation of WorkerModel\n");
        return NULL;
    }

    memset(res_bind, 0, sizeof(res_bind));

    /*      RESULT BIND        */
    mysql_set_string_result_bind(res_bind + 0, res->first_name, sizeof(res->first_name), &first_name_len);    // first_name
    mysql_set_string_result_bind(res_bind + 1, res->second_name, sizeof(res->second_name), &second_name_len); // second_name

    if (mysql_request_f(conn, &stmt, res_bind, 
        "SELECT first_name, second_name FROM worker WHERE id = %ui ", &id)) {
        fprintf(stderr, "Error while worker selecting\n");
        free_worker(res);
        return NULL;
    }

    if (mysql_stmt_fetch(stmt) == 0) {

        res->first_name[first_name_len] = '\0';
        res->second_name[second_name_len] = '\0';
        res->id = id;
    } else {
        free_worker(res);
        res = NULL;
    }
    
    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return res;
}


WorkerModel* include_worker_roles(MYSQL *conn, WorkerModel *worker) {
    MYSQL_STMT *stmt;
    MYSQL_BIND res_bind[2];

    int i = 0;
    unsigned int role_id;
    char role_name[ROLE_NAME_MAX_SIZE];
    unsigned long role_name_len;

    /* check on worker data */
    if (worker == NULL) {
        fprintf(stderr, "include_worker_roles : worker is invalide or NULL\n");
        return worker;
    }

    /* clearing worker roles array */
    if (worker->roles != NULL) {
        free(worker->roles);
        worker->roles = NULL;
        worker->roles_count = 0;
    }
    
    memset(res_bind, 0, sizeof(res_bind));

    mysql_set_uint_result_bind(res_bind, &role_id);                                               // role_id
    mysql_set_string_result_bind(res_bind + 1, role_name, sizeof(role_name), &role_name_len);     // role_name

    if (mysql_request_f(conn, &stmt, res_bind, 
            "SELECT `role`.id, `role`.name "
            "FROM `role`, worker_role AS wr "
            "WHERE wr.role_id = `role`.id "
            "AND wr.worker_id = %ui", &worker->id)) {
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


WorkerModel* refresh_worker(MYSQL *conn, WorkerModel **worker) {
    WorkerModel *res;

    if (worker == NULL || *worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        return NULL;
    }

    if ((*worker)->id == 0 || 
        (res = select_worker_by_id(conn, (*worker)->id )) == NULL) {
        fprintf(stderr, "Error : worker is not exists in data base\n");
        return NULL;
    }

    if ((*worker)->roles_included) {
        include_worker_roles(conn, res);
    }

    free_worker(*worker);
    *worker = res;
    return res;
}


WorkerModel* update_worker(MYSQL *conn, WorkerModel *worker) {
    MYSQL_STMT *stmt;

    if (worker == NULL || worker->id == 0) {
        fprintf(stderr, "Error : worker is NULL or not exists in data base\n");
        return NULL;
    }

    if (mysql_request_f(conn, &stmt, NULL, 
            "UPDATE worker "
            "SET first_name=%s, second_name=%s "
            "WHERE id=%ui ", worker->first_name, worker->second_name, &worker->id)) {
        return worker;
    }

    if (mysql_stmt_affected_rows(stmt) == 0 && worker->is_changed) {
        fprintf(stderr, "Worker is not updated\n");
        worker = NULL;
    } else {
        worker->is_changed = 0;
    }

    mysql_stmt_close(stmt);
    return worker;
}


WorkerModel* delete_worker(MYSQL *conn, WorkerModel *worker) {
    MYSQL_STMT *stmt;

    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        return NULL;
    }
    
    if (mysql_request_f(conn, &stmt, NULL, 
        "DELETE FROM worker WHERE id = %ui ", &worker->id)) {
        return worker;
    }

    if (mysql_stmt_affected_rows(stmt) == 0) {
        fprintf(stderr, "Worker is not deleted\n");
    }

    worker->id = 0;
    worker->is_changed = 0;
    
    if (worker->roles_included) {
        worker->roles_included = 0;
        if (worker->roles != NULL) {
            free(worker->roles);
            worker->roles = NULL;
        }
        worker->roles_count = 0;
    }

    mysql_stmt_close(stmt);
    return worker;
}

WorkerModel* add_worker_role(MYSQL *conn, WorkerModel **worker, RoleModel *role) {
    MYSQL_STMT *stmt;
    unsigned int code;

    if (worker == NULL || *worker == NULL) {
        fprintf(stderr, "WorkerModel is NULL\n");
        return NULL;
    }

    if (role == NULL) {
        fprintf(stderr, "RoleModel is NULL\n");
        return NULL;
    }

    if ((*worker)->id == 0) {
        fprintf(stderr, "WorkerModel is not exist in data base\n");
        return NULL;
    }
    if (role->id == 0) {
        fprintf(stderr, "RoleModel is not exist in data base\n");
        return NULL;
    }

    if ((*worker)->is_changed) {
        fprintf(stderr, "Befor adding new role to worker\nNeed to update or refresh WorkerModel\n");
        return NULL;
    }

    if (role->is_changed) {
        fprintf(stderr, "Befor adding new role to worker\nNeed to update or refresh RoleModel\n");
        return NULL;
    }

    code = mysql_request_f(conn, &stmt, NULL, 
        "INSERT INTO worker_role(worker_id, role_id) VALUES (%ui, %ui)",  &(*worker)->id, &role->id);
    if (code == 1062) {
        fprintf(stderr, "WorkerModel already have this role\n");
        return *worker;
    } else if (code != 0) {
            
        fprintf(stderr, "Error while worker role inserting\n");
        return NULL;
    }

    if ((*worker)->roles_included) {
        refresh_worker(conn, worker);
    }

    mysql_stmt_close(stmt);
    return *worker;
}

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
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

    if (mysql_request_f(conn, &stmt, bind, "SELECT id, first_name, second_name FROM worker")) {
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

    set_queue_element_free_function(res, &free_worker);

    return res;
}

Queue* refresh_workers(MYSQL *conn, Queue *workers) {
    WorkerModel *worker;
    size_t i = get_queue_size(workers);

    while (i > 0) {
        i--;

        worker = pop_queue_element(workers);

        if (refresh_worker(conn, &worker) == NULL) {
            free_worker(worker);
        } else {
            push_queue_element(workers, worker);
        }

    }
    return workers;
}

Queue* update_workers(MYSQL *conn, Queue *workers) {
    WorkerModel *worker;
    size_t i = get_queue_size(workers);

    while (i > 0) {
        i--;

        worker = pop_queue_element(workers);

        update_worker(conn, worker);

        push_queue_element(workers, worker);
    }
    return workers;
}

void delete_workers(MYSQL *conn, Queue *workers) {
    WorkerModel *worker;

    while ((worker = pop_queue_element(workers)) != NULL) {
        delete_worker(conn, worker);
        free_worker(worker);
    }
}