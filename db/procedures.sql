

DROP PROCEDURE IF EXISTS p_create_work_default_day;
/* Creating default day for work in selected week and day */
DELIMITER $
CREATE PROCEDURE p_create_work_default_day(
       IN in_day_number TINYINT UNSIGNED,
       OUT out_work_default_day_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;

        DECLARE EXIT HANDLER
        FOR SQLEXCEPTION
        BEGIN
                DELETE FROM work_day
                WHERE work_day.id = out_work_default_day_id
                ;

                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'Error while creating work_default_day.'
                ;
        END
        ;       

        SET out_work_default_day_id = NULL;       

        INSERT INTO work_day(week_id)
        VALUES (NULL)
        ;

        SET out_work_default_day_id = LAST_INSERT_ID();

        INSERT INTO work_default_day(work_day_id, day_number)
        VALUES (out_work_default_day_id, in_day_number)
        ;
END;

$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_work_week_day;
/* Creating work day for selected weak */
DELIMITER $
CREATE PROCEDURE p_create_work_week_day(
     IN in_work_week_id INT UNSIGNED,
     IN in_date DATE,
     OUT out_work_week_day_id INT UNSIGNED
)
BEGIN
        DECLARE v_week_start_date DATE;
        
        SET out_work_week_day_id = NULL;

        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'work_week is not exists.'
                ;

                SELECT work_week.date
                INTO v_week_start_date
                FROM work_week
                WHERE work_week.id = in_work_week_id
                ;
        END;

        IF in_date < v_week_start_date OR
           TIMESTAMPDIFF(DAY, v_week_start_date, in_date) > 6 THEN
           SIGNAL SQLSTATE '45000'
           SET MESSAGE_TEXT = 'work_week_day date is not on work_week.'
           ;
        END IF;
        
        BEGIN
                DECLARE EXIT HANDLER
                FOR SQLEXCEPTION
                BEGIN
                        DELETE FROM work_day
                        WHERE work_day.id = out_work_week_day_id
                        ;

                        SIGNAL SQLSTATE '45000'
                        SET MESSAGE_TEXT = 'Error while creating work_week_day.'
                        ;
                END
                ;

                INSERT INTO work_day(week_id)
                VALUES (in_work_week_id)
                ;

                SET out_work_week_day_id = LAST_INSERT_ID();

                INSERT INTO work_week_day(work_day_id, date)
                VALUES (out_work_week_day_id, in_date)
                ;
        END;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_worker_work_default_day;
DELIMITER $
CREATE PROCEDURE p_create_worker_work_default_day (
     IN in_day_number TINYINT UNSIGNED,
     IN in_worker_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'worker do not exists.'
                ;

                SELECT id
                INTO v_check
                FROM worker
                WHERE id = in_worker_id
                ;
        END;

        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of default work day in same day for same worker */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, work_default_day AS wdd, worker_work_default_day AS wwdd
                WHERE wwdd.work_default_day_id = wd.id
                AND wdd.work_day_id = wd.id
                AND wwdd.worker_id = in_worker_id
                AND wdd.day_number = in_day_number
                ;

                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'worker_work_default_day already exists for this worker in this day.'
                   ;
                END IF;
        END;

        CALL p_create_work_default_day(in_day_number, v_work_day_id);

        INSERT INTO worker_work_default_day(work_default_day_id, worker_id)
        VALUES (v_work_day_id, in_worker_id)
        ;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_line_work_default_day;
DELIMITER $
CREATE PROCEDURE p_create_line_work_default_day (
     IN in_day_number TINYINT UNSIGNED,
     IN in_line_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        /* Check on worker existing */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'worker do not exists.'
                ;

                SELECT id
                INTO v_check
                FROM line
                WHERE id = in_line_id
                ;
        END;

        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of default work day in same day for same line */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, work_default_day AS wdd, line_work_default_day AS lwdd
                WHERE lwdd.work_default_day_id = wd.id
                AND wdd.work_day_id = wd.id
                AND lwdd.line_id = in_line_id
                AND wdd.day_number = in_day_number
                ;

                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'worker_work_default_day already exists for this line in this day.'
                   ;
                END IF;
        END;

        CALL p_create_work_default_day(in_day_number, v_work_day_id);

        INSERT INTO line_work_default_day(work_default_day_id, line_id)
        VALUES (v_work_day_id, in_line_id)
        ;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_worker_work_week_day;
DELIMITER $
CREATE PROCEDURE p_create_worker_work_week_day (
     IN in_work_week_id INT UNSIGNED,
     IN in_date DATE,
     IN in_worker_id INT UNSIGNED,
     IN in_line_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'worker do not exists.'
                ;

                SELECT id
                INTO v_check
                FROM worker
                WHERE id = in_worker_id
                ;
        END;

        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of week work day in same date for same worker */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, work_week_day AS wwd, worker_work_week_day AS wwwd
                WHERE wwwd.work_week_day_id = wd.id
                AND wwd.work_day_id = wd.id
                AND wd.week_id = in_work_week_id
                AND wwwd.worker_id = in_worker_id
                AND wwwd.line_id = in_line_id
                AND wwd.date = in_date
                ;

                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'worker_work_week_day already exists for this worker on this line in this date.'
                   ;
                END IF;
        END;

        CALL p_create_work_week_day(in_work_week_id, in_date, v_work_day_id);

        INSERT INTO worker_work_week_day(work_week_day_id, worker_id, line_id)
        VALUES (v_work_day_id, in_worker_id, in_line_id)
        ;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_line_work_week_day;
DELIMITER $
CREATE PROCEDURE p_create_line_work_week_day (
     IN in_work_week_id INT UNSIGNED,
     IN in_date DATE,
     IN in_line_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'line do not exists.'
                ;

                SELECT id
                INTO v_check
                FROM line
                WHERE id = in_line_id
                ;
        END;

        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of week work day in same date for same line */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, work_week_day AS wwd, line_work_week_day AS lwwd
                WHERE lwwd.work_week_day_id = wd.id
                AND wwd.work_day_id = wd.id
                AND wd.week_id = in_work_week_id
                AND lwwd.line_id = in_line_id
                AND wwd.date = in_date
                ;

                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'line_work_week_day already exists for this line in this date.'
                   ;
                END IF;
        END;

        CALL p_create_work_week_day(in_work_week_id, in_date, v_work_day_id);

        INSERT INTO line_work_week_day(work_week_day_id, line_id)
        VALUES (v_work_day_id, in_line_id)
        ;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_insert_work_time;
DELIMITER $
CREATE PROCEDURE p_insert_work_time(
       IN in_work_day_id INT UNSIGNED,
       IN in_start_time TIME,
       IN in_end_time TIME
)
p_insert_work_time_body: BEGIN
        DECLARE v_start_time TIME;
        DECLARE v_end_time TIME;
        DECLARE v_continue BOOLEAN DEFAULT TRUE;
        
        IF in_start_time IS NULL OR
           in_end_time IS NULL OR
           in_end_time < in_start_time THEN
           LEAVE p_insert_work_time_body
           ;
        END IF;

        /* searching on work_time collision with start_time */
        BEGIN
                /* cursor for work_time collision */
                DECLARE c_prefix_timesnap CURSOR FOR
                SELECT start_time
                FROM work_time
                WHERE (in_start_time BETWEEN start_time AND end_time)
                AND work_day_id = in_work_day_id
                ;

                /* stop loop on not found */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_continue = FALSE
                ;

                SET v_continue = TRUE;

                OPEN c_prefix_timesnap;

                l_prefix_timesnap: LOOP
                                   
                     FETCH c_prefix_timesnap
                     INTO v_start_time
                     ;

                     IF v_continue = FALSE THEN
                        LEAVE l_prefix_timesnap;
                     END IF;

                     /* switch out_start_time with founded start_time */
                     IF v_start_time < in_start_time THEN
                        SET in_start_time = v_start_time;
                     END IF;
                END LOOP l_prefix_timesnap
                ;

                CLOSE c_prefix_timesnap;
        END;

        /* searching on work_time collision with end_time */
        BEGIN
                /* cursor for work_time collision */
                DECLARE c_suffix_timesnap CURSOR FOR
                SELECT end_time
                FROM work_time
                WHERE (in_end_time BETWEEN start_time AND end_time)
                AND work_day_id = in_work_day_id
                ;

                /* stop loop on not found */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_continue = FALSE
                ;

                SET v_continue = TRUE;

                OPEN c_suffix_timesnap;

                l_suffix_timesnap: LOOP

                     FETCH c_suffix_timesnap
                     INTO v_end_time
                     ;
                     
                     IF v_continue = FALSE THEN
                        LEAVE l_suffix_timesnap;
                     END IF;
                     
                     /* switch out_end_time with founded end_time */
                     IF in_end_time < v_end_time THEN
                        SET in_end_time = v_end_time;
                     END IF;
                END LOOP l_suffix_timesnap
                ;

                CLOSE c_suffix_timesnap;
        END;

        DELETE FROM work_time
        WHERE work_day_id = in_work_day_id AND
        (start_time BETWEEN in_start_time AND in_end_time) AND
        (end_time BETWEEN in_start_time AND in_end_time)
        ;

        INSERT INTO work_time(work_day_id, start_time, end_time)
        VALUES (in_work_day_id, in_start_time, in_end_time)
        ;
END;

$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_generate_line_work_time_chunk;

/* 
   Generates and assigns work time chunks for a specific line, date, and role.
   The procedure ensures the work week and daily line schedule exist,
   splits the default time interval into smaller chunks,
   determines which workers are available and eligible,
   and assigns workers to each chunk respecting role limits.
*/
DELIMITER $
CREATE PROCEDURE p_generate_line_work_time_chunk(
       IN in_date DATE,
       IN in_id_line INT UNSIGNED,
       IN in_work_time INT UNSIGNED,
       IN in_role_id INT UNSIGNED
)
BEGIN
        DECLARE v_work_week_id INT UNSIGNED;
        DECLARE v_work_default_day_id INT UNSIGNED;
        DECLARE v_line_work_week_day_id INT UNSIGNED;
        DECLARE v_start_time TIME;
        DECLARE v_end_time TIME;
        
        /* =========================================================
                   Get existing work_week for the date
                   or create a new one if it does not exist
           ========================================================= */
        BEGIN
                /* If no week exists create it */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                BEGIN
                     INSERT INTO work_week(date)
                     VALUES (DATE_SUB(in_date, INTERVAL WEEKDAY(in_date) DAY))
                     ;

                     SET v_work_week_id = LAST_INSERT_ID();
                END;

                /* searching for work_week */
                SELECT id
                INTO v_work_week_id
                FROM work_week
                WHERE TIMESTAMPDIFF(DAY, date, in_date) BETWEEN 0 AND 6
                ;
        END;

        /* =========================================================
                   Get default work time interval for the line
                   If not found stop execution
           ========================================================= */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'work_default_day not found for this line and this time'
                ;
                
                SELECT work_default_day_id,
                       wt.start_time,
                       wt.end_time
                INTO v_work_default_day_id,
                     v_start_time,
                     v_end_time
                FROM line_work_default_day AS lwdd,
                     work_time AS wt
                WHERE lwdd.line_id = in_id_line
                AND lwdd.work_default_day_id = wt.work_day_id
                AND wt.id = in_work_time
                ;
        END;

        /* =========================================================
                   Get or create line_work_week_day
           ========================================================= */
        BEGIN
                /* Create if not exists */
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                BEGIN
                        CALL p_create_line_work_week_day(v_work_week_id, in_date, in_id_line);
                        SET v_line_work_week_day_id = LAST_INSERT_ID();
                END;
                
                /* Try to find existing record */
                SELECT lwwd.work_week_day_id
                INTO v_line_work_week_day_id
                FROM line_work_week_day AS lwwd,
                     work_week_day AS wwd
                WHERE lwwd.work_week_day_id = wwd.work_day_id
                AND lwwd.line_id = in_id_line
                AND wwd.date = in_date
                ;
        END;

        /* =========================================================
                   Prepare temporary tables
                   workers_queue         stores worker/time chunks
                   workers_queue_number  stores total available time
           ========================================================= */
        CREATE TEMPORARY TABLE IF NOT EXISTS workers_queue(
               worker_id INT UNSIGNED,
               start_time TIME,
               end_time TIME
        );
        CREATE TEMPORARY TABLE IF NOT EXISTS workers_queue_number(
               worker_id INT UNSIGNED,
               work_time TIME
        );
        
        DELETE FROM workers_queue;
        DELETE FROM workers_queue_number;

        /* =========================================================
                   Split default interval into time chunks
                   and detect which workers can work each chunk
           ========================================================= */
        BEGIN
                DECLARE v_continue BOOLEAN DEFAULT TRUE;
                DECLARE v_prev_time TIME;
                DECLARE v_current_time TIME;

                /* Cursor collects all time boundaries
                   (start & end) for this work_time */
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

                        /* Insert workers who:
                           - have required role
                           - are available in this chunk
                        */
                        INSERT INTO workers_queue(worker_id, start_time, end_time)
                        SELECT worker.id, v_prev_time, v_current_time
                        FROM worker_work_default_day AS wwdd,
                             work_default_day AS wdd,
                             worker,
                             worker_role AS wr
                        WHERE wwdd.work_default_day_id = wdd.work_day_id
                        AND wwdd.worker_id = worker.id
                        AND wr.worker_id = worker.id
                        AND wr.role_id = in_role_id
                        AND f_can_work(worker.id, v_prev_time, v_current_time) = TRUE
                        ;

                        SET v_prev_time = v_current_time;

                END LOOP l_time;

                CLOSE c_time;
        END;

        /* =========================================================
                   Calculate total available time per worker
           ========================================================= */
        INSERT INTO workers_queue_number(worker_id, work_time)
        SELECT wq_t.worker_id, SEC_TO_TIME(SUM(wq_t.work_timediff)) AS work_time
        FROM (
                SELECT worker_id, TIMEDIFF(end_time, start_time) AS work_timediff
                FROM workers_queue
             ) AS wq_t
        GROUP BY wq_t.worker_id
        ;

        /* =========================================================
                   Assign workers to chunks
                   Priority: earlier time + longer availability
           ========================================================= */
        BEGIN
                DECLARE v_worker_id INT UNSIGNED;
                DECLARE v_start_time TIME;
                DECLARE v_worker_work_week_day_id INT UNSIGNED;
                DECLARE v_continue BOOLEAN DEFAULT TRUE;
                
                DECLARE v_max_workers_number INT UNSIGNED;
                DECLARE v_chunk_workers_number INT UNSIGNED;

                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_continue = FALSE
                ;

                /* Get max workers allowed for this role */
                SELECT lr.count
                INTO v_max_workers_number
                FROM line_role AS lr
                WHERE lr.line_id = in_id_line
                AND lr.role_id = in_role_id
                ;

                l_workers_queue: LOOP

                        /* Pick best candidate */
                        SELECT workers_queue.worker_id,
                               start_time,
                               end_time
                        INTO v_worker_id,
                             v_start_time,
                             v_end_time
                        FROM workers_queue,
                             workers_queue_number
                        WHERE workers_queue.worker_id = workers_queue_number.worker_id
                        ORDER BY start_time,
                                 workers_queue_number.work_time DESC
                        LIMIT 1
                        ;

                        IF v_continue = FALSE THEN
                           LEAVE l_workers_queue;
                        END IF;

                        /* Remove this chunk from queue */
                        DELETE FROM workers_queue
                        WHERE start_time = v_start_time
                        ;

                        /* Check if chunk already full */
                        SELECT COUNT(*)
                        INTO v_chunk_workers_number
                        FROM worker_work_week_day AS wwwd,
                             work_time AS wt
                        WHERE wwwd.work_week_day_id = wt.work_day_id
                        AND wwwd.line_id = in_id_line
                        AND (v_start_time BETWEEN wt.start_time AND wt.start_time)
                        AND (v_end_time BETWEEN wt.start_time AND wt.end_time)
                        ;

                        /* Skip if already full */
                        IF v_chunk_workers_number >= v_max_workers_number THEN
                        
                           SELECT CONCAT('Chunk ',
                                         CAST(v_start_time AS CHAR),
                                         ' -> ',
                                         CAST(v_end_time AS CHAR),
                                         'for line ',
                                         CAST(in_id_line AS CHAR),
                                         ' role: ',
                                         CAST(in_role_id AS CHAR),
                                         ' already full') AS message;

                           ITERATE l_workers_queue;
                        END IF;

                        /* Ensure worker_work_week_day exists */
                        BEGIN

                           DECLARE CONTINUE HANDLER
                           FOR NOT FOUND
                           BEGIN
                               CALL p_create_worker_work_week_day(
                                    v_work_week_id,
                                    in_date,
                                    v_worker_id,
                                    in_id_line
                               );
                               SET v_worker_work_week_day_id = LAST_INSERT_ID();
                           END;

                           SELECT wwwd.work_week_day_id
                           INTO v_worker_work_week_day_id
                           FROM worker_work_week_day AS wwwd, work_week_day AS wwd
                           WHERE wwwd.work_week_day_id = wwd.work_day_id
                           AND wwwd.worker_id = v_worker_id
                           AND wwwd.line_id = in_id_line
                           AND wwd.date = in_date
                           ;

                        END;

                        /* Insert time for worker and for line */
                        CALL p_insert_work_time(
                             v_worker_work_week_day_id,
                             v_start_time,
                             v_end_time
                        );
                        CALL p_insert_work_time(
                             v_line_work_week_day_id,
                             v_start_time,
                             v_end_time
                        );

                END LOOP l_workers_queue;
        END;
END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_generate_line_work_time;
DELIMITER $
CREATE PROCEDURE p_generate_line_work_time(
       IN in_line_id INT UNSIGNED,
       IN in_work_time_id INT UNSIGNED,
       IN in_date DATE
)
BEGIN
        DECLARE v_continue BOOLEAN DEFAULT TRUE;
        DECLARE v_role_id INT UNSIGNED;
        DECLARE v_role_count SMALLINT;
        DECLARE v_role_iterator SMALLINT;

        /* =========================================================
           CURSOR TO FETCH ALL ROLES FOR THE LINE
           ========================================================= */
        DECLARE c_line_role CURSOR FOR
        SELECT role_id, count
        FROM line_role
        WHERE line_id = in_line_id
        ;

        DECLARE CONTINUE HANDLER
        FOR NOT FOUND
        SET v_continue = FALSE;

        OPEN c_line_role;

        /* =========================================================
           LOOP THROUGH EACH ROLE FOR THE LINE
           ========================================================= */
        l_line_role: LOOP

                FETCH c_line_role
                INTO v_role_id, v_role_count
                ;

                IF v_continue = FALSE THEN
                   LEAVE l_line_role;
                END IF;

                SET v_role_iterator = 0;

                /* =========================================================
                   LOOP TO ASSIGN WORKERS ACCORDING TO ROLE COUNT
                   ========================================================= */
                l_role_count: LOOP

                   IF v_role_iterator >= v_role_count THEN
                      LEAVE l_line_role;
                   END IF;

                   SET v_role_iterator = v_role_iterator + 1;
                   
                   /* Call procedure to generate work time chunks for this role */
                   CALL p_generate_line_work_time_chunk(
                        in_date,
                        in_line_id,
                        in_work_time_id,
                        v_role_id
                   );
                   
                END LOOP;
        
        END LOOP l_line_role;

        CLOSE c_line_role;
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

        /* =========================================================
           CURSOR TO FETCH ALL LINES WITH DEFAULT WORK TIMES
           FOR THE GIVEN DATE (weekday) ORDER BY TIME SUM DESC
           ========================================================= */
        DECLARE c_default_time CURSOR FOR
        SELECT lwdd.line_id,
               wt.id
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

        /* =========================================================
           LOOP THROUGH ALL LINES AND DEFAULT TIMES
           ========================================================= */
        l_default_work_days: LOOP
                
                FETCH c_default_time
                INTO v_line_id, v_work_time_id
                ;

                IF v_continue = FALSE THEN
                   LEAVE l_default_work_days;
                END IF;

                CALL p_generate_line_work_time(v_line_id, v_work_time_id, in_date);
                
        END LOOP l_default_work_days;

        CLOSE c_default_time;
END;
$
DELIMITER ;