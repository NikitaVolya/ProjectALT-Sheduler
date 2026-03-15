
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

INSERT INTO line_role(line_id, role_id, worker_count)
VALUES (1, 1, 2),
       (2, 1, 1)
;

CALL p_create_line_work_default_day(6, 1);
CALL p_insert_work_time(1, "10:00:00", "12:00:00");
CALL p_insert_work_time(1, "15:00:00", "16:00:00");

CALL p_create_line_work_default_day(6, 2);
CALL p_insert_work_time(2, "10:00:00", "14:00:00");

CALL p_create_worker_work_default_day(6, 1);
CALL p_insert_work_time(3, "8:00:00", "15:45:00");

CALL p_create_worker_work_default_day(6, 2);
CALL p_insert_work_time(4, "08:00:00", "20:00:00");

CALL p_create_worker_work_default_day(6, 3);
CALL p_insert_work_time(5, "08:00:00", "13:00:00");
CALL p_insert_work_time(5, "15:45:00", "20:00:00");


CALL p_generate_day_work_time("2026-02-15");


SELECT wwd.line_work_day_id, wwd.work_day_id, worker_id, line_id, role_id, start_time, end_time
FROM worker_work_day AS wwd, work_time AS wt
WHERE wwd.work_day_id = wt.work_day_id
ORDER BY work_day_id, start_time
;

SELECT *
FROM line_work_day, work_time
WHERE line_work_day.work_day_id = work_time.work_day_id
ORDER BY id, start_time;

CALL p_remove_line_work_day_work_time(6, "11:00:00", "15:30:00");

SELECT wwd.line_work_day_id, wwd.work_day_id, worker_id, line_id, role_id, start_time, end_time
FROM worker_work_day AS wwd, work_time AS wt
WHERE wwd.work_day_id = wt.work_day_id
ORDER BY work_day_id, start_time
;

SELECT *
FROM line_work_day, work_time
WHERE line_work_day.work_day_id = work_time.work_day_id
ORDER BY id, start_time;