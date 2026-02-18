#include"mysql_base_functions.h"

/* =========================== */
/*         RESULT BINDS        */
/* =========================== */
void mysql_set_uint_result_bind(MYSQL_BIND *bind, unsigned int *id) {
    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = id;
}

void mysql_set_string_result_bind(MYSQL_BIND *bind, char *s, unsigned long buffer_size, unsigned long *s_size) {
    bind->buffer_type = MYSQL_TYPE_STRING;
    bind->buffer = s;
    bind->buffer_length = buffer_size;
    bind->length = s_size;
}


/* =========================== */
/*          PROPS BINDS        */
/* =========================== */
void mysql_set_string_prop_bind(MYSQL_BIND *bind, char *s) {
    bind->buffer_type = MYSQL_TYPE_STRING;
    bind->buffer = s;
    bind->buffer_length = strlen(s);
}

void mysql_set_uint_prop_bind(MYSQL_BIND *bind, unsigned int *value) {
    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = value;
    bind->is_unsigned = 1;
}


/* =========================== */
/*            REAQUESTS        */
/* =========================== */

int mysql_request(MYSQL *conn, MYSQL_STMT **stmt, 
                  const char *query, 
                  MYSQL_BIND *res_bind, 
                  MYSQL_BIND *prop_bind) {

    *stmt = mysql_stmt_init(conn);
    if (!*stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return 1;
    }

    if (mysql_stmt_prepare(*stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    if (prop_bind != NULL && mysql_stmt_bind_param(*stmt, prop_bind)) {
        fprintf(stderr, "Bind failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    if (mysql_stmt_execute(*stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    if (res_bind != NULL && mysql_stmt_bind_result(*stmt, res_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    if (mysql_stmt_store_result(*stmt)) {
        fprintf(stderr, "mysql_stmt_store_result() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    return 0;
}

MYSQL* mysql_create_connection(const char *address, int port, 
                               const char *database, const char *user, const char *passowrd) {
    MYSQL *conn;

    if (!(conn = mysql_init(0))) {
        fprintf(stderr, "unable to initialize connection struct\n");
        exit(EXIT_FAILURE);
    }

    if (!mysql_real_connect(conn, address, user, passowrd, database, port, NULL, 0)) {
        printf("Connection failed: %s\n", mysql_error(conn));
        exit(EXIT_FAILURE);
    }

    return conn;
}