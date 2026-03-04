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
    REQUESTF_RESULT *request_result;

    if ((res = create_worker("", "")) == NULL) {
        fprintf(stderr, "Error while memory alocation of WorkerModel\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn, 
        "SELECT first_name, second_name FROM worker WHERE id = %ui ", &id,
        MYSQL_BIND_STRING, MYSQL_BIND_STRING);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while worker selecting\n");
        free_worker(res);
        free_requestf_result(request_result);
        return NULL;
    }

    if (requestf_result_fetch(request_result,
        WORKER_FIRST_NAME_MAX_SIZE, res->first_name,
        WORKER_SECOND_NAME_MAX_SIZE, res->second_name) == 0) {
        res->id = id;
    } else {
        free_worker(res);
        res = NULL;
    }
    
    free_requestf_result(request_result);

    return res;
}


WorkerModel* include_worker_roles(MYSQL *conn, WorkerModel *worker) {
    REQUESTF_RESULT *request_result;
    int i = 0;

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

    request_result = mysql_request_f_result(conn,
            "SELECT `role`.id, `role`.name "
            "FROM `role`, worker_role AS wr "
            "WHERE wr.role_id = `role`.id "
            "AND wr.worker_id = %ui", &worker->id,
            MYSQL_BIND_UINT, MYSQL_BIND_STRING);

    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return worker;
    }

    worker->roles_count = get_requestf_num_rows(request_result);

    if (worker->roles_count != 0 && 
        (worker->roles = (RoleModel*) malloc(sizeof(RoleModel) * worker->roles_count)) == NULL) {
        free_requestf_result(request_result);
        return worker;
    }

    i = 0;
    while (requestf_result_fetch(request_result,
           &worker->roles[i].id,
           ROLE_NAME_MAX_SIZE, worker->roles[i].name) == 0) {
        i++;
    }

    worker->roles_included = 1;
    free_requestf_result(request_result);

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
    REQUESTF_RESULT *request_result;

    if (worker == NULL || worker->id == 0) {
        fprintf(stderr, "Error : worker is NULL or not exists in data base\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn,
        "UPDATE worker "
        "SET first_name=%s, second_name=%s "
        "WHERE id=%ui ", worker->first_name, worker->second_name, &worker->id);
    
    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return worker;
    }

    worker->is_changed = 0;

    free_requestf_result(request_result);
    return worker;
}


WorkerModel* delete_worker(MYSQL *conn, WorkerModel *worker) {
    REQUESTF_RESULT *request_result;

    if (worker == NULL) {
        fprintf(stderr, "Error : worker is NULL\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn,
        "DELETE FROM worker WHERE id = %ui ", &worker->id);
    
    if (get_requestf_code(request_result) != 0) {
        return worker;
    }

    if (get_requestf_affected_rows(request_result) == 0) {
        fprintf(stderr, "Worker is not deleted\n");
    } else {

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
    }

    free_requestf_result(request_result);
    return worker;
}

WorkerModel* add_worker_role(MYSQL *conn, WorkerModel **worker, RoleModel *role) {
    REQUESTF_RESULT *request_result;

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

    request_result = mysql_request_f_result(conn,
        "INSERT INTO worker_role(worker_id, role_id) VALUES (%ui, %ui)",  &(*worker)->id, &role->id);
    if (get_requestf_code(request_result) == 1062) {
        fprintf(stderr, "WorkerModel already have this role\n");
        free_requestf_result(request_result);
        return NULL;
    } else if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while worker role inserting\n");
        free_requestf_result(request_result);
        return NULL;
    }

    if ((*worker)->roles_included) {
        refresh_worker(conn, worker);
    }

    free_requestf_result(request_result);
    return *worker;
}

WorkerModel* remove_worker_role(MYSQL *conn, WorkerModel **worker, RoleModel *role) {
    REQUESTF_RESULT *request_result;

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

    request_result = mysql_request_f_result(conn,
        "DELETE FROM worker_role "
        "WHERE worker_id=%ui AND role_id=%ui",  &(*worker)->id, &role->id);
    
    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while worker RoleModel removing from WorkerModel\n");
        free_requestf_result(request_result);
        return NULL;
    }
    if (get_requestf_affected_rows(request_result) == 0) {
        fprintf(stderr, "RoleModel is not removed from WorkerModel\n");
        free_requestf_result(request_result);
        return NULL;
    }

    if ((*worker)->roles_included) {
        refresh_worker(conn, worker);
    }

    free_requestf_result(request_result);
    return *worker;
}

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_workers(MYSQL *conn) {
    REQUESTF_RESULT *result;

    WorkerModel tmp, *new_worker;
    Queue *res;

    memset(&tmp, 0, sizeof(WorkerModel));

    if ((res = create_queue()) == NULL) {
        return NULL;
    }

    result = mysql_request_f_result(conn, 
        "SELECT id, first_name, second_name FROM worker",
        MYSQL_BIND_UINT, MYSQL_BIND_STRING, MYSQL_BIND_STRING);

    if (get_requestf_code(result) != 0) {
        free_requestf_result(result);
        return NULL;
    }

    while (requestf_result_fetch(result,
        &tmp.id, 
        WORKER_FIRST_NAME_MAX_SIZE, tmp.first_name,
        WORKER_SECOND_NAME_MAX_SIZE, tmp.second_name) == 0) {
        
        if ((new_worker = create_worker_copy(&tmp)) != NULL) {
            push_queue_element(res, new_worker);
        }
    }

    set_queue_element_free_function(res, &free_worker);

    free_requestf_result(result);
    return res;
}

Queue* select_workers_by_role(MYSQL *conn, RoleModel *role) {

    REQUESTF_RESULT *result;

    WorkerModel tmp, *new_worker;
    Queue *res;

    memset(&tmp, 0, sizeof(WorkerModel));

    if (role == NULL) {
        fprintf(stderr, "RoleModel is NULL\n");
        return NULL;
    }

    if (role->id == 0) {
        fprintf(stderr, "RoleModel is not exist in data base\n");
        return NULL;
    }

    if ((res = create_queue()) == NULL) {
        return NULL;
    }

    result = mysql_request_f_result(conn, 
        "SELECT worker.id, worker.first_name, worker.second_name "
        "FROM worker, worker_role "
        "WHERE worker_role.worker_id = worker.id "
        "AND worker_role.role_id = %ui", &role->id,
        MYSQL_BIND_UINT, MYSQL_BIND_STRING, MYSQL_BIND_STRING);

    if (get_requestf_code(result) != 0) {
        free_requestf_result(result);
        return NULL;
    }

    while (requestf_result_fetch(result,
        &tmp.id, 
        WORKER_FIRST_NAME_MAX_SIZE, tmp.first_name,
        WORKER_SECOND_NAME_MAX_SIZE, tmp.second_name) == 0) {
        
        if ((new_worker = create_worker_copy(&tmp)) != NULL) {
            push_queue_element(res, new_worker);
        }
    }

    set_queue_element_free_function(res, &free_worker);

    free_requestf_result(result);
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