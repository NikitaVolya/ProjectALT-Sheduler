#ifndef _BASE_FUNCTIONS_H_
#define _BASE_FUNCTIONS_H_

#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../utils.h"

typedef enum {
    MYSQL_BIND_NONE = 0,
    MYSQL_BIND_STRING,
    MYSQL_BIND_UINT,
    MYSQL_BIND_SHORT
} MYSQL_RBIND_TYPE;

typedef struct {
    MYSQL_BIND *result_binds, *prop_binds;
    char *query;
    int result_binds_count, prop_binds_count, result_stored;
    my_bool *result_is_null;
    unsigned int code;
    MYSQL_STMT *stmt;
} REQUESTF_RESULT;

/* ======================================== */
/*         REQUESTF_RESULT FUNCTIONS        */
/* ======================================== */
REQUESTF_RESULT* create_requestf_result();

int requestf_result_fetch(REQUESTF_RESULT *value, ...);

unsigned int get_requestf_code(REQUESTF_RESULT *value);

unsigned int get_requestf_insert_id(REQUESTF_RESULT *value);

size_t get_requestf_num_rows(REQUESTF_RESULT *value);

size_t get_requestf_affected_rows(REQUESTF_RESULT *value);

void free_requestf_result(REQUESTF_RESULT *value);

/* =========================== */
/*         RESULT BINDS        */
/* =========================== */
void mysql_set_uint_result_bind(MYSQL_BIND *bind, unsigned int *id);

void mysql_set_string_result_bind(MYSQL_BIND *bind, char *s, unsigned long buffer_size, unsigned long *s_size);

void mysql_set_short_result_bind(MYSQL_BIND *bind, short *value);

/* =========================== */
/*          PROPS BINDS        */
/* =========================== */
void mysql_set_string_prop_bind(MYSQL_BIND *bind, char *s);

void mysql_set_uint_prop_bind(MYSQL_BIND *bind, unsigned int *value);

void mysql_set_short_prop_bind(MYSQL_BIND *bind, short *value);

/* =========================== */
/*            REAQUESTS        */
/* =========================== */
int mysql_request_f(MYSQL *conn, MYSQL_STMT **stmt, MYSQL_BIND *res_bind, const char *query, ...);

REQUESTF_RESULT* mysql_request_f_result(MYSQL *conn, const char *query, ...);

MYSQL* mysql_create_connection(const char *address, int port, 
                               const char *database, const char *user, const char *passowrd);

#endif /* _BASE_FUNCTIONS_H_ */