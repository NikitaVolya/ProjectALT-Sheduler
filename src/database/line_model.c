#include "line_model.h"

/* ================================ */
/*                                  */
/*       LineModel Constructot      */
/*                                  */
/* ================================ */
LineModel* create_line(const char *name, const char *product_name) {
    LineModel *res;

    if ((res = (LineModel*) malloc(sizeof(LineModel))) == NULL) {
        fprintf(stderr, "Error while memory alocation for LineModel");
        return NULL;
    }

    res->id = 0;
    
    strcpy_s(res->name, name, LINE_NAME_MAX_SIZE);
    strcpy_s(res->product_name, product_name, LINE_PRODUCT_NAME_MAX_SIZE);

    res->is_changed = 0;

    res->roles = NULL;
    res->roles_count = 0;
    res->roles_included = 0;

    return res;
}

void* create_line_copy(void *value) {
    LineModel *res, *line;
    size_t i;

    line = value;

    if ((res = create_line(line->name, line->product_name)) == NULL)
        return NULL;

    res->id = line->id;
    res->is_changed = line->is_changed;

    if (line->roles_included) {
        res->roles_count = line->roles_count;

        if ((res->roles = (LineRoleCount*) malloc(sizeof(LineRoleCount) * res->roles_count)) == NULL) {
            fprintf(stderr, "Error while memory alocation for LineModel roles\n");
            free_line(res);
            return NULL;
        } 

        for (i = 0; i < res->roles_count; i++) {
            res->roles[i] = line->roles[i];
        }

        res->roles_included = 1;
    }

    return res;
}

/* ================================ */
/*                                  */
/*        LineModel Setters         */
/*                                  */
/* ================================ */
void set_line_name(LineModel *line, const char *value) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        return;
    }

    strcpy_s(line->name, value, LINE_NAME_MAX_SIZE);

    line->is_changed = 1;
}

void set_line_product_name(LineModel *line, const char *value) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        return;
    }

    strcpy_s(line->product_name, value, LINE_PRODUCT_NAME_MAX_SIZE);

    line->is_changed = 1;
}

/* ================================ */
/*                                  */
/*        LineModel Getters         */
/*                                  */
/* ================================ */
const char* get_line_name(LineModel *line) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        exit(EXIT_FAILURE);
    }

    return line->name;
}

const char* get_line_product_name(LineModel *line) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        exit(EXIT_FAILURE);
    }

    return line->product_name;
}

const LineRoleCount* get_line_roles(LineModel *line, size_t index) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (index >= line->roles_count) {
        fprintf(stderr, "Error : MineRoleCount index out of range\n");
        exit(EXIT_FAILURE);
    }

    return &line->roles[index];
}

size_t get_line_roles_count(LineModel *line) {
    if (line == NULL) {
        fprintf(stderr, "Error : LineModel is NULL\n");
        exit(EXIT_FAILURE);
    }

    return line->roles_count;
}

/* ================================ */
/*                                  */
/*        LineModel functions       */
/*                                  */
/* ================================ */
void fprint_line(FILE *file, const LineModel *line) {
    size_t i;

    if (line == NULL) {
        printf("LineModel is NULL\n");
    } else {
        fprintf(file, "< LineModel %d : %s => %s [ ", line->id, line->name, line->product_name);

        for (i = 0; i < line->roles_count; i++) {
            if (i != 0)
                fprintf(file, ", ");
            fprintf(file, "(%s, %d)", line->roles[i].name, line->roles[i].count);
        }
        fprintf(file, " ] >\n");
    }
}

void print_line(const LineModel *role) {
    fprint_line(stdout, role);
}

/* ================================ */
/*                                  */
/*      LineModel Destructor        */
/*                                  */
/* ================================ */
void free_line(void *value) {
    LineModel *line = value;


    if (line == NULL) {
        fprintf(stderr, "LineModel is NULL\n");
    } else {

        if (line->roles_included)
            free(line->roles);
        free(line);
    }
}

/* ================================ */
/*                                  */
/*       LineModel FUNCTIONS        */
/*                                  */
/* ================================ */
LineModel* add_line(MYSQL *conn, LineModel* line) {
    MYSQL_STMT *stmt;
    unsigned int code;
    
    if (line == NULL || line->id != 0) {
        fprintf(stderr, "Error : LineModel is NULL or not exist in data base\n");
        return NULL;
    }

    code = mysql_request_f(conn, &stmt, NULL, 
                           "INSERT INTO line(name, product) VALUES (%s, %s)", 
                            line->name, line->product_name);
    if (code == 1062) {
        fprintf(stderr, "LineModel with this name already exist\n");
        return NULL;
    }
    else if (code != 0) {
        fprintf(stderr, "Error while inserting LineModel\n");
        return NULL;
    }
    line->id = mysql_stmt_insert_id(stmt);
    line->is_changed = 0;
    
    mysql_stmt_close(stmt);
    return line;
}

LineModel* select_line_by_id(MYSQL *conn, unsigned int id) {
    LineModel *res;
    MYSQL_STMT *stmt;
    
    /* BINDS FOR REQUEST */
    MYSQL_BIND res_bind[2];
    unsigned long name_len, product_len;

    if ((res = create_line("", "")) == NULL) {
        fprintf(stderr, "Error while memory alocation of LineModel\n");
        return NULL;
    }

    /*      RESULT BIND        */
    mysql_set_string_result_bind(res_bind + 0, res->name, sizeof(res->name), &name_len);                  // first_name
    mysql_set_string_result_bind(res_bind + 1, res->product_name, sizeof(res->product_name), &product_len); // second_name

    if (mysql_request_f(conn, &stmt, res_bind, 
        "SELECT name, product FROM line WHERE id = %ui ", &id)) {
        fprintf(stderr, "Error while worker selecting\n");
        free_line(res);
        return NULL;
    }

    if (mysql_stmt_fetch(stmt) == 0) {

        res->name[name_len] = '\0';
        res->product_name[product_len] = '\0';
        res->id = id;
    } else {
        free_line(res);
        res = NULL;
    }
    
    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return res;
}

LineModel* include_line_roles(MYSQL *conn, LineModel *line) {
    MYSQL_STMT *stmt;
    MYSQL_BIND res_bind[3];

    int i = 0;
    unsigned int role_id;
    char role_name[ROLE_NAME_MAX_SIZE];
    unsigned long role_name_len;
    short role_count;

    /* check on line data */
    if (line == NULL) {
        fprintf(stderr, "include_line_roles : line is invalide or NULL\n");
        return line;
    }

    /* clearing line roles array */
    if (line->roles_included) {
        free(line->roles);
        line->roles = NULL;
        line->roles_count = 0;
        line->roles_included = 0;
    }

    mysql_set_uint_result_bind(res_bind, &role_id);                                               // role_id
    mysql_set_string_result_bind(res_bind + 1, role_name, sizeof(role_name), &role_name_len);     // role_name
    mysql_set_short_prop_bind(res_bind + 2, &role_count);                                        // role_count

    if (mysql_request_f(conn, &stmt, res_bind, 
            "SELECT r.id, r.name, line_role.worker_count "
            "FROM line_role, `role` AS r "
            "WHERE line_role.role_id = r.id "
            "AND line_role.line_id = %ui "
            , &line->id)) {
        return line;
    }

    line->roles_count = mysql_stmt_num_rows(stmt);
    if (line->roles_count != 0 && 
        (line->roles = (LineRoleCount*) malloc(sizeof(LineRoleCount) * line->roles_count)) == NULL) {
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return line;
    }

    while (mysql_stmt_fetch(stmt) == 0) {

        role_name[role_name_len] = '\0';
        
        line->roles[i].id = role_id;
        strcpy(line->roles[i].name, role_name);
        line->roles[i].count = role_count;

        i++;
    }

    line->roles_included = 1;

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return line;
}

LineModel* refresh_line(MYSQL *conn, LineModel **line) {
    LineModel *res;

    if (line == NULL || *line == NULL) {
        fprintf(stderr, "Error : line is NULL\n");
        return NULL;
    }

    if ((*line)->id == 0 || 
        (res = select_line_by_id(conn, (*line)->id )) == NULL) {
        fprintf(stderr, "Error : line is not exists in data base\n");
        return NULL;
    }

    if ((*line)->roles_included) {
        include_line_roles(conn, res);
    }

    free_line(*line);
    *line = res;
    return res;
}


LineModel* update_line(MYSQL *conn, LineModel *line) {
    REQUESTF_RESULT *request_result;

    if (line == NULL || line->id == 0) {
        fprintf(stderr, "Error : line is NULL or not exists in data base\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn,
        "UPDATE line "
        "SET name=%s, product=%s "
        "WHERE id=%ui ", line->name, line->product_name, &line->id);
    
    if (get_requestf_code(request_result) != 0) {
        free_requestf_result(request_result);
        return NULL;
    }

    line->is_changed = 0;

    free_requestf_result(request_result);
    return line;
}

LineModel* delete_line(MYSQL *conn, LineModel *line) {
    REQUESTF_RESULT *request_result;

    if (line == NULL) {
        fprintf(stderr, "Error : line is NULL\n");
        return NULL;
    }

    request_result = mysql_request_f_result(conn,
        "DELETE FROM line WHERE id = %ui ", &line->id);
    
    if (get_requestf_code(request_result) != 0) {
        return NULL;
    }

    if (get_requestf_affected_rows(request_result) == 0) {
        fprintf(stderr, "LineModel is not deleted\n");
    } else {

        line->id = 0;
        line->is_changed = 0;
        
        if (line->roles_included) {
            line->roles_included = 0;
            if (line->roles != NULL) {
                free(line->roles);
                line->roles = NULL;
            }
            line->roles_count = 0;
        }
    }

    free_requestf_result(request_result);
    return line;
}

/* ====================================== */
/*                                        */
/*        QUEUE DATABASE FUNCTIONS        */
/*                                        */
/* ====================================== */
Queue* select_lines(MYSQL *conn) {
    MYSQL_BIND bind[3];
    MYSQL_STMT *stmt;

    unsigned int id;
    char name[LINE_NAME_MAX_SIZE], product_name[LINE_PRODUCT_NAME_MAX_SIZE];
    unsigned long name_len, product_len;

    LineModel *tmp;
    Queue *res;

    if ((res = create_queue()) == NULL) {
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return NULL;
    }

    mysql_set_uint_result_bind(bind, &id);                                                      // id
    mysql_set_string_result_bind(bind + 1, name, sizeof(name), &name_len);    // first_name
    mysql_set_string_result_bind(bind + 2, product_name, sizeof(product_name), &product_len); // second_name

    if (mysql_request_f(conn, &stmt, bind, "SELECT id, name, product FROM line")) {
        return NULL;
    }

    while (mysql_stmt_fetch(stmt) == 0) {

        name[name_len] = '\0';
        product_name[product_len] = '\0';

        if ((tmp = create_line(name, product_name)) != NULL) {
            tmp->id = id;
            push_queue_element(res, tmp);
        }
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    set_queue_element_free_function(res, &free_line);

    return res;
}

Queue* refresh_lines(MYSQL *conn, Queue *lines) {
    LineModel *line;
    size_t i = get_queue_size(lines);

    while (i > 0) {
        i--;

        line = pop_queue_element(lines);

        if (refresh_line(conn, &line) == NULL) {
            free_line(line);
        } else {
            push_queue_element(lines, line);
        }

    }
    return lines;
}

Queue* update_lines(MYSQL *conn, Queue *lines) {
    LineModel *line;
    size_t i = get_queue_size(lines);

    while (i > 0) {
        i--;

        line = pop_queue_element(lines);

        update_line(conn, line);

        push_queue_element(lines, line);
    }
    return lines;
}

void delete_lines(MYSQL *conn, Queue *lines) {
    LineModel *line;

    while ((line = pop_queue_element(lines)) != NULL) {
        delete_line(conn, line);
        free_line(line);
    }
}