#ifndef _WORKER_WORK_DAY_H_
#define _WORKER_WORK_DAY_H_

#include "worker_model.h"
#include "line_model.h"
#include "work_time_list.h"
#include "queue.h"


typedef struct {
    unsigned int id, week_id;
    MYSQL_TIME date;

    unsigned int worker_id;
    WorkerModel *worker;

    unsigned int line_id;
    LineModel *line;

    WorkTimeList* work_time_list;
} WorkerWorkDayModel;


/* ================================ */
/*                                  */
/*   WorkerWorkDayModel Getters     */
/*                                  */
/* ================================ */
unsigned int get_worker_work_day_id(WorkerWorkDayModel *wwd);

unsigned int get_worker_work_day_week_id(WorkerWorkDayModel *wwd);

MYSQL_TIME get_worker_work_date(WorkerWorkDayModel *wwd);

WorkerModel* get_worker_work_day_worker(WorkerWorkDayModel *wwd);

LineModel* get_worker_work_day_line(WorkerWorkDayModel *wwd);

WorkTimeList* get_worker_work_day_work_time(WorkerWorkDayModel *wwd);


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel functions    */
/*                                  */
/* ================================ */
void fprint_worker_work_day(FILE *file, WorkerWorkDayModel *wwd);

void print_worker_work_day(WorkerWorkDayModel *wwd);


/* ================================ */
/*                                  */
/*  WorkerWorkDayModel Destructor   */
/*                                  */
/* ================================ */
void free_worker_work_day(void *wwd);

/* ================================ */
/*                                  */
/*       DATABASE FUNCTIONS         */
/*                                  */
/* ================================ */
WorkerWorkDayModel* select_worker_work_day_by_id(MYSQL *conn, unsigned int id);

WorkerWorkDayModel* include_worker(MYSQL *conn, WorkerWorkDayModel *wwd);

WorkerWorkDayModel* include_line(MYSQL *conn, WorkerWorkDayModel *wwd);

WorkerWorkDayModel* include_work_time_list(MYSQL *conn, WorkerWorkDayModel *wwd);


#endif /* _WORKER_WORK_DAY_H_ */