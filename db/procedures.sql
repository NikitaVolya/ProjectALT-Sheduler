

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