#include"line_work_day_model.h"

void check_line_work_day_on_null(const char *fname, LineWorkDayModel *lwd) {
    if (lwd == NULL) {
        fprintf(stderr, "Error while executing %s: LineWorkDayModel is NULL\n", fname);
        exit(EXIT_FAILURE);
    }
}

/* ================================ */
/*                                  */
/*  LineWorkDayModel Constructot    */
/*                                  */
/* ================================ */
LineWorkDayModel* create_line_work_day_model(MYSQL *conn, MYSQL_TIME date, unsigned int line_id) {
    LineModel *line;
    REQUESTF_RESULT *request_result;

    if ((line = select_line_by_id(conn, line_id)) == NULL) {
        fprintf(stderr, "Line not found\n");
        free_line(line);
        return NULL;
    }
    
    free_line(line);
    
    request_result = mysql_request_f_result(conn, 
        "CALL p_create_line_work_day(%dt, %ui) ", &date, &line_id,
        MYSQL_BIND_UINT, MYSQL_BIND_UINT, MYSQL_BIND_DATE);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while line_work_day selecting\n");
        free_requestf_result(request_result);
        return NULL;
    }

    free_requestf_result(request_result);

    return select_line_work_day_by_line_id_date(conn, line_id, date);
}

LineWorkDayModel* create_line_work_day_modelby_line_model(MYSQL *conn, MYSQL_TIME date, LineModel *line) {

    if (line->is_changed) {
        fprintf(stderr, "Error, refresh or update line_model before use create_line_work_day_modelby_line_model.");
        return NULL;
    }

    return create_line_work_day_model(conn, date, line->id);
}

/* ================================ */
/*                                  */
/*  LineWorkDayModel Getters        */
/*                                  */
/* ================================ */
unsigned int get_line_work_day_id(LineWorkDayModel *lwd) {
    check_line_work_day_on_null("get_line_work_day_id", lwd);
    return lwd->id;
}

unsigned int get_line_work_day_week_id(LineWorkDayModel *lwd) {
    check_line_work_day_on_null("get_line_work_day_week_id", lwd);
    return lwd->week_id;
}

MYSQL_TIME get_line_work_date(LineWorkDayModel *lwd) {
    check_line_work_day_on_null("get_line_work_date", lwd);
    return lwd->date;
}

LineModel* get_line_work_day_line(LineWorkDayModel *lwd) {
    check_line_work_day_on_null("get_line_work_day_line", lwd);
    return lwd->line;
}

WorkTimeList* get_line_work_day_work_time(LineWorkDayModel *lwd) {
    check_line_work_day_on_null("get_line_work_day_work_time", lwd);
    return lwd->work_time_list;
}

/* ================================ */
/*                                  */
/*  LineWorkDayModel functions      */
/*                                  */
/* ================================ */
void fprint_line_work_day(FILE *file, LineWorkDayModel *lwd) {
    if (lwd == NULL) {
        fprintf(file, "LineWorkDayModel is NULL\n");
    } else {
        fprintf(file, "< LineWorkDayModel: %d, week_id: %d", lwd->id, lwd->week_id);
        if (lwd->line != NULL) {
            printf(" (\n");
            if (lwd->line != NULL) {
                printf("\t");
                fprint_line(file, lwd->line);
            }
            fprintf(file, ") >\n");
        } else {
            fprintf(file, ">\n");
        }
    }
}

void print_line_work_day(LineWorkDayModel *lwd) {
    fprint_line_work_day(stdout, lwd);
}


/* ================================ */
/*                                  */
/*  LineWorkDayModel Destructor    */
/*                                  */
/* ================================ */
void free_line_work_day(void *value) {
    LineWorkDayModel *lwd = (LineWorkDayModel*) value;

    if (lwd == NULL)
        return;

    if (lwd->line != NULL)
        free_line(lwd->line);
    if (lwd->work_time_list != NULL)
        free_work_time_list(lwd->work_time_list);

    free(lwd);
}

/* ================================ */
/*                                  */
/*       DATABASE FUNCTIONS         */
/*                                  */
/* ================================ */
LineWorkDayModel* select_line_work_day_by_id(MYSQL *conn, unsigned int id) {
    LineWorkDayModel *res;
    REQUESTF_RESULT *request_result;

    if ((res = (LineWorkDayModel*) malloc(sizeof(LineWorkDayModel))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        return NULL;
    }

    res->line = NULL;
    res->work_time_list = NULL;

    request_result = mysql_request_f_result(conn, 
        "SELECT wd.week_id, lwd.line_id, wd.date "
        "FROM line_work_day AS lwd, work_day AS wd "
        "WHERE lwd.work_day_id = wd.id AND wd.id = %ui ", &id,
        MYSQL_BIND_UINT, MYSQL_BIND_UINT, MYSQL_BIND_DATE);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while line_work_day selecting\n");
        free_line_work_day(res);
        free_requestf_result(request_result);
        return NULL;
    }

    if (requestf_result_fetch(request_result,
        &res->week_id, &res->line_id, &res->date) == 0) {
        res->id = id;
    } else {
        free_line_work_day(res);
        res = NULL;
    }
    
    free_requestf_result(request_result);

    return res;
}

LineWorkDayModel* select_line_work_day_by_line_id_date(MYSQL *conn, unsigned int line_id, MYSQL_TIME date) {
    LineWorkDayModel *res;
    REQUESTF_RESULT *request_result;

    if ((res = (LineWorkDayModel*) malloc(sizeof(LineWorkDayModel))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        return NULL;
    }

    res->line = NULL;
    res->work_time_list = NULL;

    request_result = mysql_request_f_result(conn, 
        "SELECT lwd.work_day_id, wd.week_id "
        "FROM line_work_day AS lwd, work_day AS wd "
        "WHERE lwd.work_day_id = wd.id AND lwd.line_id = %ui AND wd.date = %dt ", &line_id, &date,
        MYSQL_BIND_UINT, MYSQL_BIND_UINT);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while line_work_day selecting\n");
        free_line_work_day(res);
        free_requestf_result(request_result);
        return NULL;
    }

    if (requestf_result_fetch(request_result,
        &res->id, &res->week_id) == 0) {
        res->line_id = line_id;
        res->date = date;
    } else {
        free_line_work_day(res);
        res = NULL;
    }
    
    free_requestf_result(request_result);

    return res;
}

LineWorkDayModel* include_line_work_day_line(MYSQL *conn, LineWorkDayModel *lwd) {
    check_line_work_day_on_null("include_line_work_day_line", lwd);

    if (lwd->id == 0)
        return NULL;
    
    if (lwd->line != NULL) {
        free_line(lwd->line);
        lwd->line = NULL;
    }

    if ((lwd->line = select_line_by_id(conn, lwd->line_id)) == NULL) {
        fprintf(stderr, "Error while including line in LineWorkDayModel\n");
        return NULL;
    }

    return lwd;
}


LineWorkDayModel* include_line_work_day_work_time_list(MYSQL *conn, LineWorkDayModel *lwd) {
    check_line_work_day_on_null("include_line_work_day_work_time_list", lwd);

    if (lwd->id == 0)
        return NULL;

    if ((lwd->work_time_list = select_work_time(conn, lwd->id)) == NULL) {
        return NULL;
    }

    return lwd;
}