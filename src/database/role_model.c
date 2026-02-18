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
