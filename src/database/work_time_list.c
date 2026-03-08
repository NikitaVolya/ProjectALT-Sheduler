#include "work_time_list.h"


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel functions    */
/*                                  */
/* ================================ */
WorkTimeList* select_work_time(MYSQL *conn, unsigned int work_day_id) {
    WorkTimeList *res;
    REQUESTF_RESULT *result;
    MYSQL_TIME start_time, end_time;
    size_t i;

    if ((res = (WorkTimeList*) malloc(sizeof(WorkTimeList))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        return NULL;
    }

    result = mysql_request_f_result(conn, 
        "SELECT start_time, end_time "
        "FROM work_time "
        "WHERE work_day_id = %ui "
        "ORDER BY start_time ", &work_day_id,
        MYSQL_BIND_TIME, MYSQL_BIND_TIME
    );

    if (get_requestf_code(result) != 0) {
        free_requestf_result(result);
        free(res);
        return NULL;
    }

    res->work_day_id = work_day_id;
    res->start_times = NULL;
    res->end_times = NULL;
    res->count = get_requestf_num_rows(result);

    if (res->count == 0) {
        free_requestf_result(result);
        return res;
    }

    if ((res->start_times = (MYSQL_TIME*) malloc(sizeof(MYSQL_TIME) * res->count)) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        free_requestf_result(result);
        free(res);
        return NULL;
    }

    if ((res->end_times = (MYSQL_TIME*) malloc(sizeof(MYSQL_TIME) * res->count)) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        free_requestf_result(result);
        free(res->start_times);
        free(res);
        return NULL;
    }

    i = 0;

    while (requestf_result_fetch(result, &start_time, &end_time) == 0) {
        res->start_times[i] = start_time;
        res->end_times[i] = end_time;
        i++;
    }
    
    free_requestf_result(result);
    return res;
}

/* ================================ */
/*                                  */
/*   WorkerWorkDayModel Getters     */
/*                                  */
/* ================================ */
inline void check_on_null(const char *fname, WorkTimeList *work_time_list) {
    if (work_time_list == NULL) {
        fprintf(stderr, "Error while executing %s: WorkTimeList is NULL\n", fname);
        exit(EXIT_FAILURE);
    }
}

inline void check_index_on_outofbounds(const char *fname, size_t index, size_t count) {
    if (index >= count) {
        fprintf(stderr, "Error while executing %s: index out of range\n", fname);
        exit(EXIT_FAILURE);
    }
}

MYSQL_TIME get_work_time_list_start_time(WorkTimeList *work_time_list, size_t index) {
    check_on_null("get_work_time_list_start_time", work_time_list);
    check_index_on_outofbounds("get_work_time_list_start_time", index, work_time_list->count);
    return work_time_list->start_times[index];
}

MYSQL_TIME get_work_time_list_end_time(WorkTimeList *work_time_list, size_t index)  {
    check_on_null("get_work_time_list_end_time", work_time_list);
    check_index_on_outofbounds("get_work_time_list_end_time", index, work_time_list->count);
    return work_time_list->end_times[index];
}

size_t get_work_time_list_count(WorkTimeList *work_time_list) {
    check_on_null("get_work_time_list_count", work_time_list);
    return work_time_list->count;
}

/* ================================ */
/*                                  */
/*  WorkerWorkDayModel Destructor   */
/*                                  */
/* ================================ */
void free_work_time_list(void *value) {
    WorkTimeList *work_time_list;

    if (value != NULL) {
        work_time_list = (WorkTimeList*) value;
        free(work_time_list->start_times);
        free(work_time_list->end_times);
        free(work_time_list);
    }
}