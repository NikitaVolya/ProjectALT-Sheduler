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

/* ================================ */
/*                                  */
/*        WorkerModel Setters       */
/*                                  */
/* ================================ */
void set_role_name(RoleModel *role, const char *value);

/* ================================ */
/*                                  */
/*        WorkerModel Getters       */
/*                                  */
/* ================================ */
const char* get_role_name(const RoleModel *role);

#endif /* _ROLE_MODEL_H_ */