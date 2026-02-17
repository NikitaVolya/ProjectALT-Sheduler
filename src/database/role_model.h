#ifndef _ROLE_MODEL_H_
#define _ROLE_MODEL_H_

#define ROLE_NAME_MAX_SIZE 256

typedef struct {
    unsigned int id;
    char name[ROLE_NAME_MAX_SIZE];
} RoleModel;



#endif /* _ROLE_MODEL_H_ */