
source delete.sql
source creation.sql
source procedures.sql
source function.sql
source triggers.sql

INSERT INTO worker(first_name, second_name)
VALUES ('test', 'worker'),
       ('second', 'worker'),
       ('reste', 'worker')
;

INSERT INTO role(name)
VALUES ('test')
;

INSERT INTO worker_role(worker_id, role_id)
VALUES (1, 1),
       (2, 1),
       (3, 1)
;

INSERT INTO line(name, product)
VALUES ('A1', 'toy'),
       ('B1', 'toy')
;

INSERT INTO line_role(line_id, role_id, count)
VALUES (1, 1, 2),
       (2, 1, 1)
;

CALL p_create_line_work_default_day(6, 1);
CALL p_insert_work_time(1, "10:00:00", "12:00:00");
CALL p_insert_work_time(1, "15:00:00", "16:00:00");

CALL p_create_line_work_default_day(6, 2);

CALL p_create_worker_work_default_day(6, 1);
CALL p_insert_work_time(3, "8:00:00", "15:45:00");

CALL p_create_worker_work_default_day(6, 2);
CALL p_insert_work_time(4, "08:00:00", "20:00:00");

CALL p_create_worker_work_default_day(6, 3);
CALL p_insert_work_time(5, "08:00:00", "13:00:00");
CALL p_insert_work_time(5, "15:45:00", "20:00:00");


CALL p_generate_day_work_time("2026-02-15");


SELECT wwwd.work_week_day_id AS work_day_id, worker_id, line_id, start_time, end_time
FROM worker_work_week_day AS wwwd, work_time AS wt
WHERE wwwd.work_week_day_id = wt.work_day_id
ORDER BY start_time
;

SELECT *
FROM line_work_week_day, work_time
WHERE line_work_week_day.work_week_day_id = work_time.work_day_id
ORDER BY start_time;