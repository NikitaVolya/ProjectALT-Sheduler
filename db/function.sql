

DROP FUNCTION IF EXISTS f_get_wdd_day_number;

DELIMITER $
CREATE FUNCTION f_get_wdd_day_number(
       in_work_day_id INT UNSIGNED
)
RETURNS TINYINT UNSIGNED
READS SQL DATA
BEGIN
        RETURN (SELECT day_number FROM work_default_day WHERE work_day_id = in_work_day_id);
END;

$
DELIMITER ;


DROP FUNCTION IF EXISTS f_get_wd_work_week_id;

DELIMITER $
CREATE FUNCTION f_get_wd_work_week_id(
       in_work_day_id INT UNSIGNED
)
RETURNS TINYINT UNSIGNED
READS SQL DATA
BEGIN
        RETURN (SELECT work_week_id FROM work_day WHERE id = in_id_work_day_id);
END;

$
DELIMITER ;


DROP FUNCTION IF EXISTS f_can_work;

DELIMITER $
CREATE FUNCTION f_can_work(
       in_worker_id INT UNSIGNED,
       in_start_time TIME,
       in_end_time TIME
)
RETURNS BOOLEAN
READS SQL DATA
BEGIN
        /* check if user have this time in his default work time */
        IF (SELECT COUNT(wwdd.work_default_day_id)
            FROM worker_work_default_day AS wwdd, work_time AS wt
            WHERE wt.work_day_id = wwdd.work_default_day_id
            AND wwdd.worker_id = in_worker_id
            AND (in_start_time BETWEEN wt.start_time AND wt.end_time)
            AND (in_end_time BETWEEN wt.start_time AND wt.end_time)) = 0 THEN
           RETURN FALSE;
        END IF;

        /* check is user dosen't work in this time */
        IF (SELECT COUNT(wwwd.work_week_day_id)
                 FROM worker_work_week_day AS wwwd, work_time AS wt
                 WHERE wt.work_day_id = wwwd.work_week_day_id
                 AND wwwd.worker_id = in_worker_id
                 AND (in_start_time BETWEEN wt.start_time AND wt.end_time)
                 AND (in_end_time BETWEEN wt.start_time AND wt.end_time)) <> 0 THEN
           RETURN FALSE;
        END IF;

        RETURN TRUE;
END;
$
DELIMITER ;

DROP FUNCTION IF EXISTS f_is_enough_workers;

DELIMITER $
CREATE FUNCTION f_is_enough_workers(
       in_line_id INT UNSIGNED,
       in_role_id INT UNSIGNED,
       in_start_time TIME,
       in_end_time TIME
)
RETURNS BOOLEAN
READS SQL DATA
BEGIN
        DECLARE v_worker_count INT UNSIGNED;
        DECLARE v_role_count INT UNSIGNED;

        SET v_worker_count = (SELECT COUNT(wwwd.id)
           FROM worker_work_week_day AS wwwd, work_time AS wt, worker_role AS wr
           WHERE wwwd.work_week_day_id = wt.work_day_id
           AND wwwd.worker_id = wr.worker_id
           AND wr.role_id = in_role_id
           AND (in_start_time BETWEEN wt.start_time AND wt.end_time)
           AND (in_end_time BETWEEN wt.start_time AND wt.end_time))
        ;

        SET v_role_count = (SELECT (count)
            FROM line_role
            WHERE line_id = in_line_id)
        ;

        RETURN v_worker_count = v_role_count;
END;
$
DELIMITER ;
