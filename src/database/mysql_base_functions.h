#ifndef _BASE_FUNCTIONS_H_
#define _BASE_FUNCTIONS_H_

#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../utils.h"

typedef enum {
    MYSQL_RBIND_STRING,
    MYSQL_RBIND_UINT,
    MYSQL_RBIND_SHORT
} MYSQL_RBIND_TYPE;

typedef struct {
    MYSQL_BIND *result_binds, *prop_binds;
    int result_binds_count, prop_binds_count;
    MYSQL_STMT *stmt;
} REQUESTF_RESULT;

/* ======================================== */
/*         REQUESTF_RESULT FUNCTIONS        */
/* ======================================== */
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


MYSQL* mysql_create_connection(const char *address, int port, 
                               const char *database, const char *user, const char *passowrd);

#endif /* _BASE_FUNCTIONS_H_ */