#ifndef _LINE_MODEL_H_
#define _LINE_MODEL_H_

#define LINE_NAME_MAX_SIZE 101
#define LINE_PRODUCT_NAME_MAX_SIZE 101

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql_base_functions.h"
#include "role_model.h"
#include "queue.h"

#include "../utils.h"


typedef struct {
    unsigned int id;
    char name[ROLE_NAME_MAX_SIZE];
    short count;
} LineRoleCount;

typedef struct {
    unsigned int id;
    char name[LINE_NAME_MAX_SIZE];
    char product_name[LINE_PRODUCT_NAME_MAX_SIZE];

    LineRoleCount *roles;
    size_t roles_count;
    short roles_included;

    short is_changed;
} LineModel;

/* ================================ */
/*                                  */
/*       LineModel Constructot      */
/*                                  */
/* ================================ */
LineModel* create_line(const char *name, const char *product_name);

void* create_line_copy(void *value);

/* ================================ */
/*                                  */
/*        RoleModel Setters         */
/*                                  */
/* ================================ */
void set_line_name(LineModel *line, const char *value);

void set_line_product_name(LineModel *line, const char *value);

/* ================================ */
/*                                  */
/*        RoleModel Getters         */
/*                                  */
/* ================================ */
const char* get_line_name(LineModel *line);

const char* get_line_product_name(LineModel *line);

const LineRoleCount* get_line_roles(LineModel *line, size_t index);

size_t get_line_roles_count(LineModel *line);

/* ================================ */
/*                                  */
/*        RoleModel functions       */
/*                                  */
/* ================================ */
void fprint_line(FILE *file, const LineModel *role);

void print_line(const LineModel *role);

/* ================================ */
/*                                  */
/*      LineModel Destructor        */
/*                                  */
/* ================================ */
void free_line(void *value);

/* ================================ */
/*                                  */
/*       DATABASE FUNCTIONS         */
/*                                  */
/* ================================ */
LineModel* add_line(MYSQL *conn, LineModel *line);

LineModel* select_line_by_id(MYSQL *conn, unsigned int id);

LineModel* include_line_roles(MYSQL *conn, LineModel *line);

LineModel* refresh_line(MYSQL *conn, LineModel **line);

LineModel* update_line(MYSQL *conn, LineModel *line);

LineModel* delete_line(MYSQL *conn, LineModel *line);

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_lines(MYSQL *conn);

Queue* refresh_lines(MYSQL *conn, Queue *lines);

Queue* update_lines(MYSQL *conn, Queue *lines);

void delete_workers(MYSQL *conn, Queue *lines);

#endif /* _LINE_MODEL_H_ */