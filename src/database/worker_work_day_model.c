#include"worker_work_day_model.h"

void check_worker_work_day_on_null(const char *fname, WorkerWorkDayModel *wwd) {
    if (wwd == NULL) {
        fprintf(stderr, "Error while executing %s: WorkerWorkDayModel is NULL\n", fname);
        exit(EXIT_FAILURE);
    }
}

/* ================================ */
/*                                  */
/*   WorkerWorkDayModel Getters     */
/*                                  */
/* ================================ */
unsigned int get_worker_work_day_id(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_day_id", wwd);
    return wwd->id;
}

unsigned int get_worker_work_day_week_id(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_day_week_id", wwd);
    return wwd->week_id;
}

MYSQL_TIME get_worker_work_date(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_date", wwd);
    return wwd->date;
}

WorkerModel* get_worker_work_day_worker(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_day_worker", wwd);
    return wwd->worker;
}

LineModel* get_worker_work_day_line(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_day_line", wwd);
    return wwd->line;
}

WorkTimeList* get_worker_work_day_work_time(WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("get_worker_work_day_work_time", wwd);
    return wwd->work_time_list;
}

/* ================================ */
/*                                  */
/*        RoleModel functions       */
/*                                  */
/* ================================ */
void fprint_worker_work_day(FILE *file, WorkerWorkDayModel *wwd) {
    if (wwd == NULL) {
        fprintf(file, "WorkerWorkDayModel is NULL\n");
    } else {
        fprintf(file, "< WorkerWorkDayModel: %d, week_id: %d", wwd->id, wwd->week_id);
        if (wwd->worker != NULL || wwd->line != NULL) {
            printf(" (\n");
            if (wwd->worker != NULL) {
                printf("\t");
                fprint_worker(file, wwd->worker);
            }
            if (wwd->line != NULL) {
                printf("\t");
                fprint_line(file, wwd->line);
            }
            fprintf(file, ") >\n");
        } else {
            fprintf(file, ">\n");
        }
    }
}

void print_worker_work_day(WorkerWorkDayModel *wwd) {
    fprint_worker_work_day(stdout, wwd);
}


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel Destructor   */
/*                                  */
/* ================================ */
void free_worker_work_day(void *value) {
    WorkerWorkDayModel *wwd = (WorkerWorkDayModel*) value;

    if (wwd == NULL)
        return;

    if (wwd->line != NULL)
        free_line(wwd->line);
    if (wwd->worker != NULL)
        free_worker(wwd->worker);
    if (wwd->work_time_list != NULL)
        free_work_time_list(wwd->work_time_list);

    free(wwd);
}

/* ================================ */
/*                                  */
/*       DATABASE FUNCTIONS         */
/*                                  */
/* ================================ */
WorkerWorkDayModel* select_worker_work_day_by_id(MYSQL *conn, unsigned int id) {
    WorkerWorkDayModel *res;
    REQUESTF_RESULT *request_result;

    if ((res = (WorkerWorkDayModel*) malloc(sizeof(WorkerWorkDayModel))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        return NULL;
    }

    res->worker = NULL;
    res->line = NULL;
    res->work_time_list = NULL;

    request_result = mysql_request_f_result(conn, 
        "SELECT wd.week_id, wwd.worker_id, wwd.line_id, wd.date "
        "FROM worker_work_day AS wwd, work_day AS wd "
        "WHERE wwd.work_day_id = wd.id AND wd.id = %ui ", &id,
        MYSQL_BIND_UINT, MYSQL_BIND_UINT, MYSQL_BIND_UINT, MYSQL_BIND_DATE);

    if (get_requestf_code(request_result) != 0) {
        fprintf(stderr, "Error while worker_work_day selecting\n");
        free_worker_work_day(res);
        free_requestf_result(request_result);
        return NULL;
    }

    if (requestf_result_fetch(request_result,
        &res->week_id, &res->worker_id, &res->line_id, &res->date) == 0) {
        res->id = id;
    } else {
        free_worker_work_day(res);
        res = NULL;
    }
    
    free_requestf_result(request_result);

    return res;
}

WorkerWorkDayModel* include_worker(MYSQL *conn, WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("include_worker", wwd);

    if (wwd->id == 0)
        return NULL;

    if (wwd->worker != NULL) {
        free_worker(wwd->worker);
        wwd->worker = NULL;
    }

    if ((wwd->worker = select_worker_by_id(conn, wwd->worker_id)) == NULL) {
        fprintf(stderr, "Error while including worker in WorkerWorkDayModel\n");
        return NULL;
    }

    return wwd;
}

WorkerWorkDayModel* include_line(MYSQL *conn, WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("include_line", wwd);

    if (wwd->id == 0)
        return NULL;
    
    if (wwd->line != NULL) {
        free_line(wwd->line);
        wwd->line = NULL;
    }

    if ((wwd->line = select_line_by_id(conn, wwd->line_id)) == NULL) {
        fprintf(stderr, "Error while including line in WorkerWorkDayModel\n");
        return NULL;
    }

    return wwd;
}


WorkerWorkDayModel* include_work_time_list(MYSQL *conn, WorkerWorkDayModel *wwd) {
    check_worker_work_day_on_null("include_work_time_list", wwd);

    if (wwd->id == 0)
        return NULL;

    if ((wwd->work_time_list = select_work_time(conn, wwd->id)) == NULL) {
        return NULL;
    }

    return wwd;
}