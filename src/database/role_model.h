#ifndef _ROLE_MODEL_H_
#define _ROLE_MODEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql_base_functions.h"
#include "queue.h"

#include "../utils.h"

#define ROLE_NAME_MAX_SIZE 256


typedef struct {
    unsigned int id;
    char name[ROLE_NAME_MAX_SIZE];
    
    short is_changed;
} RoleModel;


/* ================================ */
/*                                  */
/*       RoleModel Constructot      */
/*                                  */
/* ================================ */
RoleModel* create_role(const char *name);

void* create_role_copy(void *value);

/* ================================ */
/*                                  */
/*        RoleModel Setters         */
/*                                  */
/* ================================ */
void set_role_name(RoleModel *role, const char *value);

/* ================================ */
/*                                  */
/*        RoleModel Getters         */
/*                                  */
/* ================================ */
const char* get_role_name(const RoleModel *role);

unsigned int get_role_id(const RoleModel *role);

/* ================================ */
/*                                  */
/*        RoleModel functions       */
/*                                  */
/* ================================ */
void fprint_role(FILE *file, const RoleModel *role);

void print_role(const RoleModel *role);

/* ================================ */
/*                                  */
/*       RoleModel destructor       */
/*                                  */
/* ================================ */
void free_role(void *value);

/* ================================ */
/*                                  */
/*        DATABASE FUNCTIONS        */
/*                                  */
/* ================================ */
RoleModel* add_role(MYSQL *conn, RoleModel* role);

RoleModel* select_role_by_id(MYSQL *conn, unsigned int id);

RoleModel* refresh_role(MYSQL *conn, RoleModel **role);

RoleModel* update_role(MYSQL *conn, RoleModel *role);

RoleModel* delete_role(MYSQL *conn, RoleModel *role);

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_roles(MYSQL *conn);

Queue* refresh_roles(MYSQL *conn, Queue *roles);

Queue* update_roles(MYSQL *conn, Queue *roles);

void delete_roles(MYSQL *conn, Queue *roles);

#endif /* _ROLE_MODEL_H_ */