#ifndef _LINE_WORK_DAY_H_
#define _LINE_WORK_DAY_H_

#include "worker_model.h"
#include "line_model.h"
#include "work_time_list.h"
#include "queue.h"


typedef struct {
    unsigned int id, week_id;
    MYSQL_TIME date;

    unsigned int line_id;
    LineModel *line;

    WorkTimeList* work_time_list;
} LineWorkDayModel;

/* ================================ */
/*                                  */
/*       RoleModel Constructot      */
/*                                  */
/* ================================ */
LineWorkDayModel* create_line_work_day_model(MYSQL *conn, MYSQL_TIME date, unsigned int line_id);

LineWorkDayModel* create_line_work_day_modelby_line_model(MYSQL *conn, MYSQL_TIME date, LineModel *line);

/* ================================ */
/*                                  */
/*   LineWorkDayModel Getters       */
/*                                  */
/* ================================ */
unsigned int get_line_work_day_id(LineWorkDayModel *lwd);

unsigned int get_line_work_day_week_id(LineWorkDayModel *lwd);

MYSQL_TIME get_line_work_date(LineWorkDayModel *lwd);

LineModel* get_line_work_day_line(LineWorkDayModel *lwd);

WorkTimeList* get_line_work_day_work_time(LineWorkDayModel *lwd);


/* ================================ */
/*                                  */
/*  LineWorkDayModel functions      */
/*                                  */
/* ================================ */
void fprint_line_work_day(FILE *file, LineWorkDayModel *lwd);

void print_line_work_day(LineWorkDayModel *lwd);


/* ================================ */
/*                                  */
/*  LineWorkDayModel Destructor     */
/*                                  */
/* ================================ */
void free_line_work_day(void *lwd);

/* ================================ */
/*                                  */
/*       DATABASE FUNCTIONS         */
/*                                  */
/* ================================ */
LineWorkDayModel* select_line_work_day_by_id(MYSQL *conn, unsigned int id);

LineWorkDayModel* select_line_work_day_by_line_id_date(MYSQL *conn, unsigned int line_id, MYSQL_TIME date);

LineWorkDayModel* include_line_work_day_line(MYSQL *conn, LineWorkDayModel *lwd);

LineWorkDayModel* include_line_work_day_work_time_list(MYSQL *conn, LineWorkDayModel *lwd);


#endif /* _LINE_WORK_DAY_H_ */