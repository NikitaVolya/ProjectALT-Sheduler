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
char* mysql_init_prop_binds(char *query, MYSQL_BIND **res_binds, va_list *list) {
    char *query_c, *res_query, *res_query_c;
    int counter, i;

    /* searching params in query string */
    counter = 0;
    for (query_c = query; *query_c != '\0'; query_c++) {
        if (*query_c == '%')
            counter++;
    }

    /* memory alocation of result query */
    if ((res_query = (char *) malloc(sizeof(char) * (strlen(query) + 1))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        exit(EXIT_FAILURE);
    }

    /* if no params found stop function */
    if (counter == 0) {
        *res_binds = NULL;
        strcpy(res_query, query);
        return res_query;
    }

    /* memory alocation of binds */
    if ((*res_binds = (MYSQL_BIND*) malloc(sizeof(MYSQL_BIND) * counter)) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        exit(EXIT_FAILURE);
    }
    memset(*res_binds, 0, sizeof(MYSQL_BIND) * counter);
    
    
    i = 0;
    res_query_c = res_query;

    /* parsing params */
    /* %s => string */
    /* %ui => unsigned int */
    for (query_c = query; *query_c != '\0'; query_c++) {

        if (*query_c == '%') {
            
            /* string param */
            if (query_c[1] == 's') {
                mysql_set_string_prop_bind(*res_binds + i, (char*) va_arg(*list, char*));
                query_c++;
            } 
            /* unsigned int param */
            else if (query_c[1] == 'u' && query_c[2] == 'i') {

                mysql_set_uint_prop_bind(*res_binds + i, (unsigned int*) va_arg(*list, unsigned int*));
                query_c = query_c + 2;
            } 
            /* unfounded param */
            else {
                fprintf(stderr, "Error unkown parameter %%%c%c", query_c[1], query_c[2]);
                exit(EXIT_FAILURE);
            }
            i++;

            *res_query_c = '?';
            res_query_c++;
        } 
        else {
            *res_query_c = *query_c;
            res_query_c++;
        }
    }
    res_query_c++;
    *res_query_c = '\0';

    return res_query;
}

int mysql_request_f(MYSQL *conn, MYSQL_STMT **stmt, MYSQL_BIND *res_bind, const char *query, ...) {
    char *f_query;
    MYSQL_BIND *prop_bind = NULL;
    va_list parameters;
    
    va_start(parameters, query);
    f_query = mysql_init_prop_binds((char *) query, &prop_bind, &parameters);
    va_end(parameters);

    *stmt = mysql_stmt_init(conn);
    if (!*stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return 1;
    }

    if (mysql_stmt_prepare(*stmt, f_query, strlen(f_query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        return 1;
    }

    if (prop_bind != NULL && mysql_stmt_bind_param(*stmt, prop_bind)) {
        fprintf(stderr, "Bind failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        free(prop_bind);
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

    if (prop_bind != NULL) {
        free(prop_bind);
    }

    free(f_query);

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