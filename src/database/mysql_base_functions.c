#include"mysql_base_functions.h"

/* ======================================== */
/*         REQUESTF_RESULT FUNCTIONS        */
/* ======================================== */
void requestf_result_init_query(REQUESTF_RESULT *value, char *query, va_list *list) {
    char *query_c, *res_query_c;
    int counter, i;

    /* counting params in query string */
    counter = 0;
    for (query_c = query; *query_c != '\0'; query_c++) {
        if (*query_c == '%')
            counter++;
    }

    /* memory alocation of result query */
    if ((value->query = (char *) malloc(sizeof(char) * (strlen(query) + 1))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        exit(EXIT_FAILURE);
    }

    /* if no params found stop function */
    if (counter == 0) {
        strcpy(value->query, query);
        return;
    }

    /* memory alocation of binds */
    if ((value->prop_binds = (MYSQL_BIND*) malloc(sizeof(MYSQL_BIND) * counter)) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        exit(EXIT_FAILURE);
    }
    value->prop_binds_count = counter;
    
    i = 0;
    res_query_c = value->query;

    /* parsing params */
    /* %s => string */
    /* %ui => unsigned int */
    /* %d => short */
    for (query_c = query; *query_c != '\0'; query_c++) {

        if (*query_c == '%') {
            
            /* string param */
            if (query_c[1] == 's') {
                mysql_set_string_prop_bind(value->prop_binds + i, (char*) va_arg(*list, char*));
                query_c++;
            } 
            /* short param */
            else if (query_c[1] == 'd') {
                mysql_set_short_prop_bind(value->prop_binds + i, (short*) va_arg(*list, short*));
                query_c++;
            }
            /* unsigned int param */
            else if (query_c[1] == 'u' && query_c[2] == 'i') {
                mysql_set_uint_prop_bind(value->prop_binds + i, (unsigned int*) va_arg(*list, unsigned int*));
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

    /* delete whitespace in end of query */
    res_query_c--;
    while (*res_query_c == ' ') res_query_c--;
    res_query_c++;
    *res_query_c = '\0';

    return;
}

void requestf_result_init_result_binds(REQUESTF_RESULT *value, va_list *list) {
    int i;
    MYSQL_RBIND_TYPE field_type;

    if (value == NULL || value->stmt == NULL) {
        value->result_binds = NULL;
        return; 
    }

    value->result_binds_count = mysql_stmt_field_count(value->stmt);

    if (value->result_binds_count == 0) {
        value->result_binds = NULL;
        return;
    }
    
    value->result_is_null = (my_bool*) calloc(value->result_binds_count, sizeof(my_bool));
    value->result_binds = (MYSQL_BIND*) calloc(value->result_binds_count, sizeof(MYSQL_BIND));

    if (value->result_is_null == NULL) {
        fprintf(stderr, "Error while memory allocation for result_is_null\n");
        exit(EXIT_FAILURE);
    }
    if (value->result_binds == NULL) {
        fprintf(stderr, "Error while memory allocation for result binds\n");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < value->result_binds_count; i++) {
        
        value->result_binds[i].is_null = &value->result_is_null[i];
        field_type = (MYSQL_RBIND_TYPE) va_arg(*list, MYSQL_RBIND_TYPE);

        switch (field_type)
        {
        case MYSQL_BIND_STRING:
            value->result_binds[i].buffer_type = MYSQL_TYPE_STRING;
            value->result_binds[i].buffer_length = 256;
            value->result_binds[i].buffer = malloc(sizeof(char) * 256);
            value->result_binds[i].length = malloc(sizeof(unsigned long));
            if (value->result_binds[i].length == NULL) {
                fprintf(stderr, "Error while memory allocation for result binds\n");
                exit(EXIT_FAILURE);
            }
            break;
        case MYSQL_BIND_UINT:
            value->result_binds[i].buffer_type = MYSQL_TYPE_LONG;
            value->result_binds[i].buffer = malloc(sizeof(unsigned int));
            break;
        case MYSQL_BIND_SHORT:
            value->result_binds[i].buffer_type = MYSQL_TYPE_SHORT;
            value->result_binds[i].is_unsigned = 0;
            value->result_binds[i].buffer = malloc(sizeof(short));
            break;
        default:
            fprintf(stderr, "Error invalide result bind\n");
            exit(EXIT_FAILURE);
        }

        if (value->result_binds[i].buffer == NULL) {
            fprintf(stderr, "Error while memory allocation for result binds\n");
            exit(EXIT_FAILURE);
        }
    }
}

int requestf_result_fetch(REQUESTF_RESULT *value, ...) {
    va_list parameters;
    int res, i;
    unsigned long s_length;
    void *buffer;

    va_start(parameters, value);

    if (mysql_stmt_fetch(value->stmt) == 0) {

        for (i = 0; i < value->result_binds_count; i++) {
            
            buffer = value->result_binds[i].buffer;

            switch (value->result_binds[i].buffer_type) {
                case MYSQL_TYPE_STRING:
                    s_length = *((unsigned long *) value->result_binds[i].length);
                    ((char *) buffer)[s_length] = '\0';

                    strcpy_s((char*) va_arg(parameters, char*), 
                             (char *) buffer,
                             (size_t) va_arg(parameters, size_t));
                    break;
                case MYSQL_TYPE_LONG:
                    *((unsigned int *) va_arg(parameters, unsigned int *)) = *((unsigned int*) buffer);
                    break;
                case MYSQL_TYPE_SHORT:
                    *((short *) va_arg(parameters, short*)) = *((short *) buffer);
                    break;
                default:
                    break;
            }
        }

        res = 0;
    } else {
        res = 1;
    }

    va_end(parameters);
    return res;
}

unsigned int get_requestf_code(REQUESTF_RESULT *value) {
    if (value == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS NULL\n");
        return 1;
    }
    return value->code;
}

size_t get_requestf_num_rows(REQUESTF_RESULT *value) {
    if (value == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS NULL\n");
        return 0;
    }
    if (value->stmt == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS INVALIDE\n");
        return 0;
    }
    return mysql_stmt_num_rows(value->stmt);
}
unsigned int get_requestf_insert_id(REQUESTF_RESULT *value) {
    if (value == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS NULL\n");
        return 0;
    }
    if (value->stmt == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS INVALIDE\n");
        return 0;
    }
    return mysql_stmt_insert_id(value->stmt);
}


size_t get_requestf_affected_rows(REQUESTF_RESULT *value) {
    if (value == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS NULL\n");
        return 0;
    }
    if (value->stmt == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS INVALIDE\n");
        return 0;
    }
    return mysql_stmt_affected_rows(value->stmt);
}

REQUESTF_RESULT* create_requestf_result() {
    REQUESTF_RESULT *res = NULL;

    if ((res = (REQUESTF_RESULT*) malloc(sizeof(REQUESTF_RESULT))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        exit(EXIT_FAILURE);
    }

    res->stmt = NULL;

    res->result_binds = NULL;
    res->result_binds_count = 0;

    res->prop_binds = NULL;
    res->prop_binds_count = 0;

    res->query = NULL;
    res->code = 1;

    return res;
}

void free_requestf_result(REQUESTF_RESULT *value) {
    int i;

    if (value == NULL) {
        fprintf(stderr, "REQUESTF_RESULT IS NULL\n");
        return;
    }

    if (value->result_stored == 1) {
        mysql_stmt_free_result(value->stmt);
    }

    if (value->stmt != NULL) {
        mysql_stmt_close(value->stmt);
        value->stmt = NULL;
    }

    for (i = 0; i < value->result_binds_count; i++) {
        free(value->result_binds[i].buffer);

        switch (value->result_binds[i].buffer_type) {
            case MYSQL_TYPE_STRING:
                free(value->result_binds[i].length);
                break;
            default:
                break;
        }
    }

    if (value->result_binds != NULL) {
        free(value->result_binds);
        free(value->result_is_null);
        value->result_binds = NULL;
        value->result_binds_count = 0;
    }

    if (value->prop_binds != NULL) {
        free(value->prop_binds);
        value->prop_binds = NULL;
        value->prop_binds_count = 0;
    }

    if (value->query != NULL) {
        free(value->query);
        value->query = NULL;
    }

    free(value);
}


/* =========================== */
/*         RESULT BINDS        */
/* =========================== */
void mysql_set_uint_result_bind(MYSQL_BIND *bind, unsigned int *id) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = id;
}

void mysql_set_string_result_bind(MYSQL_BIND *bind, char *s, unsigned long buffer_size, unsigned long *s_size) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind->buffer_type = MYSQL_TYPE_STRING;
    bind->buffer = s;
    bind->buffer_length = buffer_size;
    bind->length = s_size;
}

void mysql_set_short_result_bind(MYSQL_BIND *bind, short *value) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind[0].buffer_type = MYSQL_TYPE_SHORT;
    bind[0].buffer = value;
    bind[0].is_unsigned = 0;
}


/* =========================== */
/*          PROPS BINDS        */
/* =========================== */
void mysql_set_string_prop_bind(MYSQL_BIND *bind, char *s) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind->buffer_type = MYSQL_TYPE_STRING;
    bind->buffer = s;
    bind->buffer_length = strlen(s);
}

void mysql_set_uint_prop_bind(MYSQL_BIND *bind, unsigned int *value) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = value;
    bind->is_unsigned = 1;
}

void mysql_set_short_prop_bind(MYSQL_BIND *bind, short *value) {
    memset(bind, 0, sizeof(MYSQL_BIND));

    bind[0].buffer_type = MYSQL_TYPE_SHORT;
    bind[0].buffer = value;
    bind[0].is_unsigned = 0;
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
    /* %d => short */
    for (query_c = query; *query_c != '\0'; query_c++) {

        if (*query_c == '%') {
            
            /* string param */
            if (query_c[1] == 's') {
                mysql_set_string_prop_bind(*res_binds + i, (char*) va_arg(*list, char*));
                query_c++;
            } 
            /* short param */
            else if (query_c[1] == 'd') {
                mysql_set_short_prop_bind(*res_binds + i, (short*) va_arg(*list, short*));
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

    /* delete whitespace in end of query */
    res_query_c--;
    while (*res_query_c == ' ') res_query_c--;
    res_query_c++;
    *res_query_c = '\0';

    return res_query;
}

int mysql_request_f(MYSQL *conn, MYSQL_STMT **stmt, MYSQL_BIND *res_bind, const char *query, ...) {
    char *f_query;
    MYSQL_BIND *prop_bind = NULL;
    va_list parameters;
    unsigned int error_code;
    
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
        error_code = mysql_stmt_errno(*stmt);
        mysql_stmt_close(*stmt);

        if (prop_bind != NULL) {
            free(prop_bind);
        }

        return error_code;
    }

    if (res_bind != NULL && mysql_stmt_bind_result(*stmt, res_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);

        if (prop_bind != NULL) {
            free(prop_bind);
        }
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

REQUESTF_RESULT* mysql_request_f_result(MYSQL *conn, const char *query, ...) {
    va_list parameters;
    REQUESTF_RESULT *result = NULL;

    result = create_requestf_result();

    va_start(parameters, query);

    requestf_result_init_query(result, (char*) query, &parameters);

    result->stmt = mysql_stmt_init(conn);
    if (result->stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        result->code = 1;
        
        va_end(parameters);
        return result;
    }

    if (mysql_stmt_prepare(result->stmt, result->query, strlen(result->query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(result->stmt));
        result->code = 1;
        
        va_end(parameters);
        return result;
    }

    if (result->prop_binds != NULL && mysql_stmt_bind_param(result->stmt, result->prop_binds)) {
        fprintf(stderr, "Bind failed: %s\n", mysql_stmt_error(result->stmt));
        result->code =  1;
        
        va_end(parameters);
        return result;
    }

    if (mysql_stmt_execute(result->stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(result->stmt));
        result->code = mysql_stmt_errno(result->stmt);

        
        va_end(parameters);
        return result;
    }

    requestf_result_init_result_binds(result, &parameters);

    if (result->result_binds != NULL && mysql_stmt_bind_result(result->stmt, result->result_binds)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(result->stmt));
        result->code =  1;

        va_end(parameters);
        return result;
    }
    
    va_end(parameters);

    if (mysql_stmt_store_result(result->stmt)) {
        fprintf(stderr, "mysql_stmt_store_result() failed: %s\n", mysql_stmt_error(result->stmt));
        result->code =  1;
        return result;
    }

    result->result_stored = 1;
    result->code = 0;
    return result;
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