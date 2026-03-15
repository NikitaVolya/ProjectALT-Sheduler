

DROP FUNCTION IF EXISTS f_get_wdd_day_number;
/*
    Function: f_get_wdd_day_number

    Description:
        Returns the day number associated with a default work day.

        The function retrieves the `day_number` from the table
        `work_default_day` using the provided `work_day_id`.

    Parameters:
        IN in_work_day_id
            ID of the work_day record that represents
            a default work day.

    Returns:
        TINYINT UNSIGNED
            The day number (for example 0–6) corresponding
            to the weekday stored in `work_default_day`.

    Notes:
        - The function assumes that `work_day_id` exists in
          `work_default_day`.
        - If no record is found, the function returns NULL.
*/
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
/*
    Function: f_get_wd_work_week_id

    Description:
        Returns the work week ID associated with a specific work day.

        The function retrieves the `week_id` from the `work_day` table
        using the provided `work_day_id`. This allows determining
        to which work week a particular work day belongs.

    Parameters:
        IN in_work_day_id
            ID of the work_day record.

    Returns:
        INT UNSIGNED
            The identifier of the work week (`week_id`)
            that the specified work day belongs to.

    Notes:
        - The function assumes that the provided `work_day_id`
          exists in the `work_day` table.
        - If no matching record exists, the function returns NULL.
*/
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
/*
    Function: f_can_work

    Description:
        Determines whether a worker is available to work during a
        specified time interval on a given date.

        The function performs two checks:
        1. Verifies that the worker has a default work schedule
           covering the requested time interval for the given weekday.
        2. Ensures that the worker is not already assigned to work
           during the same time interval on that date.

        If both conditions are satisfied, the worker is considered
        available for the requested time slot.

    Parameters:
        IN in_worker_id
            ID of the worker whose availability is being checked.

        IN in_date
            The date for which availability is evaluated.

        IN in_start_time
            Start time of the requested work interval.

        IN in_end_time
            End time of the requested work interval.

    Returns:
        BOOLEAN
            TRUE  – worker can work in the specified interval.
            FALSE – worker cannot work (either not scheduled by default
                    or already assigned to work).
*/

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
/*
    Function: f_is_enough_workers

    Description:
        Determines whether the required number of workers for a specific
        role is already assigned to a production line during a given
        time interval.

        The function:
        1. Retrieves the line ID associated with the given line work day.
        2. Retrieves the maximum number of workers required for the role
           on that line.
        3. Counts how many workers are already assigned to the same
           line, role, and time interval.

        If the number of assigned workers is greater than or equal to
        the required number, the function returns TRUE.

    Parameters:
        IN in_line_work_day_id
            ID of the work day for the production line.

        IN in_role_id
            Role identifier for which the worker count is checked.

        IN in_start_time
            Start time of the interval being evaluated.

        IN in_end_time
            End time of the interval being evaluated.

    Returns:
        BOOLEAN
            TRUE  – enough workers are already assigned.
            FALSE – more workers are still required.

    Notes:
        - The function compares workers assigned to the same role and
          line within the provided time interval.

        - IMPORTANT:
          The function assumes that workers are assigned to the SAME
          continuous time interval. If workers cover different parts
          of the interval, the result may be inaccurate.

          Example:
              Required workers for a role: 2

              Worker A works: 08:00–10:00
              Worker B works: 10:00–12:00

              If the function is called for interval 08:00–12:00,
              it may incorrectly return TRUE (enough workers),
              even though both workers are never present at the
              same time.

        - To avoid such inaccuracies, the schedule should be divided
          into smaller time chunks and the function should be called
          for each chunk separately.
*/
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
