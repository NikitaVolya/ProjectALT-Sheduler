
source delete.sql
source creation.sql
source procedures.sql
source function.sql
source triggers.sql

INSERT INTO worker(first_name, second_name)
VALUES ('test', 'worker'),
       ('second', 'worker')
;

INSERT INTO role(name)
VALUES ('test')
;

INSERT INTO worker_role(worker_id, role_id)
VALUES (1, 1),
       (2, 1)
;

INSERT INTO line(name, product)
VALUES ('A1', 'toy'),
       ('B1', 'toy')
;

INSERT INTO line_role(line_id, role_id, count)
VALUES (1, 1, 1),
       (2, 1, 1)
;

CALL p_create_line_work_default_day(6, 1);
CALL p_insert_work_time(1, "10:00:00", "12:00:00");
CALL p_insert_work_time(1, "15:00:00", "16:00:00");

CALL p_create_line_work_default_day(6, 2);
CALL p_insert_work_time(2, "11:00:00", "15:30:00");

CALL p_create_worker_work_default_day(6, 1);
CALL p_insert_work_time(3, "8:00:00", "15:45:00");

CALL p_create_worker_work_default_day(6, 2);
CALL p_insert_work_time(4, "15:30:00", "20:00:00");



DROP PROCEDURE IF EXISTS p_generate_line_work_time;

DELIMITER $
CREATE PROCEDURE p_generate_line_work_time(
       IN in_date DATE,
       IN in_id_line INT UNSIGNED,
       IN in_work_time INT UNSIGNED
)
BEGIN
        DECLARE v_work_week_id INT UNSIGNED;
        DECLARE v_work_default_day_id INT UNSIGNED;
        DECLARE v_line_work_week_day_id INT UNSIGNED;
        DECLARE v_start_time TIME;
        DECLARE v_end_time TIME;
        
        /* geting work_week or creating a new one */
        BEGIN
                /* creating if not found */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                BEGIN
                     INSERT INTO work_week(date)
                     VALUES (DATE_SUB(in_date, INTERVAL WEEKDAY(in_date) DAY))
                     ;

                     SET v_work_week_id = LAST_INSERT_ID();
                END;

                /* searching */
                SELECT id
                INTO v_work_week_id
                FROM work_week
                WHERE TIMESTAMPDIFF(DAY, date, in_date) BETWEEN 0 AND 6
                ;
        END;

        /* getting work_default_day or exit for not found */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'work_default_day not found for this line and this time'
                ;
                
                SELECT work_default_day_id, wt.start_time, wt.end_time
                INTO v_work_default_day_id, v_start_time, v_end_time
                FROM line_work_default_day AS lwdd,
                     work_time AS wt
                WHERE lwdd.line_id = in_id_line
                AND lwdd.work_default_day_id = wt.work_day_id
                AND wt.id = in_work_time
                ;
        END;

        /* checking on existing line_work_week_day */
        /* if exists in this time raise error */
        /* else if exists but not in this time */
        /* else insert new line_work_week_day */
        BEGIN
                /* creating if not exists */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                BEGIN
                        CALL p_create_line_work_week_day(v_work_week_id, in_date, in_id_line);
                        SET v_line_work_week_day_id = LAST_INSERT_ID();
                END;
                

                SELECT lwwd.work_week_day_id
                INTO v_line_work_week_day_id
                FROM line_work_week_day AS lwwd,
                     work_week_day AS wwd
                WHERE lwwd.work_week_day_id = wwd.work_day_id
                AND lwwd.line_id = in_id_line
                AND wwd.date = in_date
                ;

                IF (SELECT COUNT(id)
                    FROM work_time
                    WHERE work_day_id = v_line_work_week_day_id
                    AND (start_time BETWEEN v_start_time AND v_end_time)
                    AND (end_time BETWEEN v_start_time AND v_end_time)) <> 0 THEN
                    SIGNAL SQLSTATE '45000'
                    SET MESSAGE_TEXT = 'line is already work in this time'
                    ;
                END IF;
        END;

        /* table of workers and time when they can work */
        CREATE TEMPORARY TABLE IF NOT EXISTS workers_queue(
               worker_id INT UNSIGNED,
               start_time TIME,
               end_time TIME
        );
        DELETE FROM workers_queue;

        /* table of worker enable time to work on this line */
        CREATE TEMPORARY TABLE IF NOT EXISTS workers_queue_number(
               worker_id INT UNSIGNED,
               work_time TIME
        );
        DELETE FROM workers_queue_number;

        /* searchin for workers in time chanks */
        BEGIN
                DECLARE v_continue BOOLEAN DEFAULT TRUE;
                DECLARE v_prev_time TIME;
                DECLARE v_current_time TIME;
                
                DECLARE c_time CURSOR FOR
                SELECT t.time_p
                FROM  (SELECT wt.start_time AS time_p, wdd.day_number AS day_number
                       FROM work_time AS wt, work_default_day AS wdd
                       WHERE wdd.work_day_id = wt.work_day_id
                       UNION
                       SELECT wt.end_time AS time_p, wdd.day_number AS day_number
                       FROM work_time AS wt, work_default_day AS wdd
                       WHERE wdd.work_day_id = wt.work_day_id) AS t
                WHERE t.day_number = WEEKDAY(in_date)
                AND (t.time_p BETWEEN v_start_time AND v_end_time)
                ORDER BY t.time_p
                ;
                
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_continue = FALSE;

                OPEN c_time;

                FETCH c_time
                INTO v_prev_time;

                l_time: LOOP

                        FETCH c_time
                        INTO v_current_time
                        ;

                        IF v_continue = FALSE THEN
                           LEAVE l_time;
                        END IF;

                        INSERT INTO workers_queue(worker_id, start_time, end_time)
                        SELECT worker.id, v_prev_time, v_current_time
                        FROM worker_work_default_day AS wwdd, work_default_day AS wdd, worker
                        WHERE wwdd.work_default_day_id = wdd.work_day_id
                        AND wwdd.worker_id = worker.id
                        AND f_can_work(worker.id, v_prev_time, v_current_time) = TRUE
                        ;

                        SET v_prev_time = v_current_time;

                END LOOP l_time;

                CLOSE c_time;
        END;

        INSERT INTO workers_queue_number(worker_id, work_time)
        SELECT wq_t.worker_id, SUM(wq_t.work_timediff) AS work_time
        FROM (SELECT worker_id, TIMEDIFF(end_time, start_time) AS work_timediff
              FROM workers_queue) AS wq_t
        GROUP BY wq_t.worker_id
        ;

        /* adding worker_work_day */
        BEGIN
                DECLARE v_worker_id INT UNSIGNED;
                DECLARE v_start_time TIME;
                DECLARE v_worker_work_week_day_id INT UNSIGNED;
                DECLARE v_continue BOOLEAN DEFAULT TRUE;

                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_continue = FALSE
                ;

                l_workers_queue: LOOP

                        SELECT workers_queue.worker_id, start_time, end_time
                        INTO v_worker_id, v_start_time, v_end_time
                        FROM workers_queue, workers_queue_number
                        WHERE workers_queue.worker_id = workers_queue_number.worker_id
                        ORDER BY start_time, workers_queue_number.work_time DESC
                        LIMIT 1
                        ;

                        IF v_continue = FALSE THEN
                           LEAVE l_workers_queue;
                        END IF;
                        
                        DELETE FROM workers_queue
                        WHERE start_time = v_start_time
                        ;

                        /* creating worker_work_week_day if not exists */
                        /* saving worker_work_week_day_id */
                        BEGIN

                           DECLARE CONTINUE HANDLER
                           FOR NOT FOUND
                           BEGIN
                               CALL p_create_worker_work_week_day(v_work_week_id, in_date, v_worker_id, in_id_line);
                               SET v_worker_work_week_day_id = LAST_INSERT_ID();
                           END;

                           /* selecting */
                           SELECT wwwd.work_week_day_id
                           INTO v_worker_work_week_day_id
                           FROM worker_work_week_day AS wwwd, work_week_day AS wwd
                           WHERE wwwd.work_week_day_id = wwd.work_day_id
                           AND wwwd.worker_id = v_worker_id
                           AND wwwd.line_id = in_id_line
                           AND wwd.date = in_date
                           ;

                        END;

                        /* inserting work time */
                        /* for worker */
                        CALL p_insert_work_time(v_worker_work_week_day_id, v_start_time, v_end_time);
                        /* for line */
                        CALL p_insert_work_time(v_line_work_week_day_id, v_start_time, v_end_time);

                END LOOP l_workers_queue;
        END;
END;
$
DELIMITER ;

DROP PROCEDURE IF EXISTS p_generate_day_work_time;
DELIMITER $
CREATE PROCEDURE p_generate_day_work_time(
       IN in_date DATE
)
BEGIN
        DECLARE v_continue BOOLEAN DEFAULT TRUE;
        DECLARE v_line_id INT UNSIGNED;
        DECLARE v_work_time_id INT UNSIGNED;

        DECLARE c_default_time CURSOR FOR
        SELECT lwdd.line_id, wt.id
        FROM line_work_default_day AS lwdd,
             work_default_day AS wdd,
             work_time AS wt
        WHERE lwdd.work_default_day_id = wt.work_day_id
        AND lwdd.work_default_day_id = wdd.work_day_id
        AND wdd.day_number = WEEKDAY(in_date)
        ORDER BY TIMEDIFF(wt.end_time, wt.start_time)
        ;

        DECLARE CONTINUE HANDLER
        FOR NOT FOUND
        SET v_continue = FALSE;

        OPEN c_default_time;

        l_default_work_days: LOOP
                
                FETCH c_default_time
                INTO v_line_id, v_work_time_id
                ;

                IF v_continue = FALSE THEN
                   LEAVE l_default_work_days;
                END IF;

                BEGIN

                        DECLARE CONTINUE HANDLER
                        FOR SQLEXCEPTION
                        SELECT CONCAT('Imosible to generate work day for ',
                                       CAST(in_date AS CHAR),
                                       ' ',
                                       (SELECT CONCAT(CAST(start_time AS CHAR),
                                                      ' -> ' ,
                                                      CAST(end_time AS CHAR))
                                        FROM work_time
                                        WHERE id = v_work_time_id
                                        LIMIT 1),
                                       ' and line ',
                                       CAST(v_line_id AS CHAR),
                                       ', because its already exists.') AS message
                                       ;

                        CALL p_generate_line_work_time(in_date, v_line_id, v_work_time_id);
                END;
                
        END LOOP l_default_work_days;

        CLOSE c_default_time;
END;
$
DELIMITER ;

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