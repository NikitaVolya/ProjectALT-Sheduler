

DROP FUNCTION IF EXISTS f_get_wdd_day_number;

DELIMITER $
CREATE FUNCTION f_get_wdd_day_number(
       in_work_day_id INT UNSIGNED
)
RETURNS TINYINT UNSIGNED
READS SQL DATA
BEGIN
        RETURN (
                SELECT day_number 
                FROM work_default_day 
                WHERE work_day_id = in_work_day_id
                );
END;

$
DELIMITER ;


DROP FUNCTION IF EXISTS f_get_wd_work_week_id;
DELIMITER $
CREATE FUNCTION f_get_wd_work_week_id(
       in_work_day_id INT UNSIGNED
)
RETURNS INT UNSIGNED
READS SQL DATA
BEGIN
        RETURN (
            SELECT week_id
            FROM work_day
            WHERE id = in_work_day_id
        );
END;
$
DELIMITER ;


DROP FUNCTION IF EXISTS f_can_work;

DELIMITER $
CREATE FUNCTION f_can_work(
       in_worker_id INT UNSIGNED,
       in_date DATE,
       in_start_time TIME,
       in_end_time TIME
)
RETURNS BOOLEAN
READS SQL DATA
BEGIN
        DECLARE v_day_number TINYINT UNSIGNED DEFAULT WEEKDAY(in_date);


        /* Check if worker has default work time */
        IF (
            SELECT COUNT(*)
            FROM work_time
            WHERE work_day_id IN (
                SELECT work_default_day_id
                FROM worker_work_default_day, work_default_day
                WHERE worker_id = in_worker_id
                AND worker_work_default_day.work_default_day_id = work_default_day.work_day_id
                AND work_default_day.day_number = v_day_number
            )
            AND in_start_time BETWEEN start_time AND end_time
            AND in_end_time BETWEEN start_time AND end_time
        ) = 0 THEN
           RETURN FALSE;
        END IF;

        /* Check if worker already works at this time */
        IF (
            SELECT COUNT(*)
            FROM work_time
            WHERE work_day_id IN (
                SELECT work_day_id
                FROM worker_work_day, work_day
                WHERE worker_id = in_worker_id
                AND work_day.date = in_date
            )
            AND in_start_time BETWEEN start_time AND end_time
            AND in_end_time BETWEEN start_time AND end_time
        ) <> 0 THEN
           RETURN FALSE;
        END IF;

        RETURN TRUE;
END;
$
DELIMITER ;


DROP FUNCTION IF EXISTS f_is_enough_workers;

DELIMITER $
CREATE FUNCTION f_is_enough_workers(
       in_line_work_day_id INT UNSIGNED,
       in_role_id INT UNSIGNED,
       in_start_time TIME,
       in_end_time TIME
)
RETURNS BOOLEAN
READS SQL DATA
BEGIN
        DECLARE v_line_id INT UNSIGNED;
        DECLARE v_max_role_count INT UNSIGNED;
        DECLARE v_role_count INT UNSIGNED;


        /* get line id */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'line_work_day not found';

                SELECT line_id
                INTO v_line_id
                FROM line_work_day
                WHERE work_day_id = in_line_work_day_id;
        END;


        /* get required worker count for role */
        BEGIN  
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_max_role_count = 0;

                SELECT worker_count
                INTO v_max_role_count
                FROM line_role
                WHERE line_id = v_line_id
                AND role_id = in_role_id;
        END;

        /* count workers already assigned */
        SELECT COUNT(*)
        INTO v_role_count
        FROM worker_work_day
        WHERE line_work_day_id = in_line_work_day_id
        AND role_id = in_role_id
        AND work_day_id IN (
                SELECT work_day_id
                FROM work_time
                WHERE in_start_time BETWEEN start_time AND end_time
                AND in_end_time BETWEEN start_time AND end_time
        );

        RETURN v_role_count >= v_max_role_count;
END;
$
DELIMITER ;
