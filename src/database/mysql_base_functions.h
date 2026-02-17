#ifndef _BASE_FUNCTIONS_H_
#define _BASE_FUNCTIONS_H_

#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================== */
/*         RESULT BINDS        */
/* =========================== */
void mysql_set_uint_result_bind(MYSQL_BIND *bind, unsigned int *id);

void mysql_set_string_result_bind(MYSQL_BIND *bind, char *s, unsigned long buffer_size, unsigned long *s_size);

/* =========================== */
/*          PROPS BINDS        */
/* =========================== */
void mysql_set_string_prop_bind(MYSQL_BIND *bind, char *s);

void mysql_set_uint_prop_bind(MYSQL_BIND *bind, unsigned int *value);

/* =========================== */
/*            REAQUESTS        */
/* =========================== */
int mysql_request(MYSQL *conn, MYSQL_STMT **stmt, 
                  const char *query, 
                  MYSQL_BIND *res_bind, 
                  MYSQL_BIND *prop_bind);

#endif /* _BASE_FUNCTIONS_H_ */