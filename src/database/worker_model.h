#ifndef _WORKER_MODEL_H_
#define _WORKER_MODEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql_base_functions.h"
#include "queue.h"

#include "role_model.h"

#include "../utils.h"

#define WORKER_FIRST_NAME_MAX_SIZE 101
#define WORKER_SECOND_NAME_MAX_SIZE 101

typedef struct {
    unsigned int id;
    char first_name[WORKER_FIRST_NAME_MAX_SIZE], 
         second_name[WORKER_SECOND_NAME_MAX_SIZE];
    
    short is_changed;

    RoleModel *roles;
    size_t roles_count;
    short roles_included;
} WorkerModel;

/* ================================ */
/*                                  */
/*      WorkerModel Constructot     */
/*                                  */
/* ================================ */
WorkerModel* create_worker(const char *first_name, const char *second_name);

/* ================================ */
/*                                  */
/*        WorkerModel Setters       */
/*                                  */
/* ================================ */
void set_worker_first_name(WorkerModel *worker, const char *value);

void set_worker_last_name(WorkerModel *worker, const char *value);

/* ================================ */
/*                                  */
/*        WorkerModel Getters       */
/*                                  */
/* ================================ */
unsigned int get_worker_id(WorkerModel *worker);

const char* get_worker_first_name(WorkerModel *worker);

const char* get_worker_second_name(WorkerModel *worker);

const RoleModel* get_worker_role(WorkerModel *worker, size_t index);

size_t get_worker_roles_count(WorkerModel *worker);

/* ================================ */
/*                                  */
/*      WorkerModel Destructor      */
/*                                  */
/* ================================ */
void free_worker(WorkerModel* worker);


/* ================================ */
/*                                  */
/*        DATABASE FUNCTIONS        */
/*                                  */
/* ================================ */
WorkerModel* add_worker(MYSQL *conn, WorkerModel* worker);

WorkerModel* select_worker_by_id(MYSQL *conn, unsigned int id);

Queue* select_workers(MYSQL *conn);

WorkerModel* include_worker_roles(MYSQL *conn, WorkerModel *worker);

WorkerModel* refresh_worker(MYSQL *conn, WorkerModel **worker);

WorkerModel* update_worker(MYSQL *conn, WorkerModel *worker);

WorkerModel* delete_worker(MYSQL *conn, WorkerModel *worker);

#endif /* _WORKER_MODEL_H_ */