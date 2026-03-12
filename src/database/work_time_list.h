#ifndef _WORK_TIME_LIST_H_
#define _WORK_TIME_LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql_base_functions.h"

typedef struct {
    unsigned int work_day_id;
    MYSQL_TIME *start_times, *end_times;
    size_t count;
} WorkTimeList;


/* ================================ */
/*                                  */
/*   WorkerWorkDayModel Getters     */
/*                                  */
/* ================================ */
MYSQL_TIME get_work_time_list_start_time(WorkTimeList *work_time_list, size_t index);

MYSQL_TIME get_work_time_list_end_time(WorkTimeList *work_time_list, size_t index);

size_t get_work_time_list_count(WorkTimeList *work_time_list);


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel Destructor   */
/*                                  */
/* ================================ */
void free_work_time_list(void *value);


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel functions    */
/*                                  */
/* ================================ */
WorkTimeList* select_work_time(MYSQL *conn, unsigned int work_day_id);

#endif /* _WORK_TIME_LIST_H_ */