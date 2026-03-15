

DROP PROCEDURE IF EXISTS p_create_work_default_day;

/*
    Procedure: p_create_work_default_day

    Description:
        Creates a new default work day for a specific day of the week.

        The procedure performs two steps:
        1. Creates a record in the table `work_day`.
        2. Creates a corresponding record in `work_default_day`
           that links the created work_day with a day number.

        If an error occurs while creating the default work day,
        the previously created `work_day` record will be removed.

    Parameters:
        IN  in_day_number
            Number of the day in the week (for example 0–6).

        OUT out_work_default_day_id
            Returns the ID of the created work_day record.
            Returns NULL if the procedure fails.

    Error handling:
        If any SQL exception occurs while creating `work_default_day`,
        the procedure:
            - deletes the previously created record from `work_day`
            - raises a custom error message.
*/

DELIMITER $
CREATE PROCEDURE p_create_work_default_day(
       IN in_day_number TINYINT UNSIGNED,
       OUT out_work_default_day_id INT UNSIGNED
)
BEGIN
        /* 
           Error handler:
           If any SQL error occurs during the procedure execution,
           remove the created work_day record to avoid orphan data.
        */
        DECLARE EXIT HANDLER
        FOR SQLEXCEPTION
        BEGIN   
                /* Rollback created work_day record */
                DELETE FROM work_day
                WHERE work_day.id = out_work_default_day_id
                ;

                /* message */
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'Error while creating work_default_day.'
                ;
        END
        ;       

        /* Default output value (in case creation fails) */
        SET out_work_default_day_id = NULL;       

        /* Create base work_day record */
        INSERT INTO work_day(week_id)
        VALUES (NULL)
        ;

        /* Save generated ID */
        SET out_work_default_day_id = LAST_INSERT_ID();

        /* Create default day linked to the created work_day */
        INSERT INTO work_default_day(work_day_id, day_number)
        VALUES (out_work_default_day_id, in_day_number)
        ;
END;

$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_work_day;
/*
    Procedure: p_create_work_day

    Description:
        Creates a work day for a specific week.

        The procedure:
        1. Checks if the specified work_week exists.
        2. Verifies that the provided date belongs to that week.
        3. Creates a record in `work_day`.
        4. Creates a corresponding record in `work_day`
           that links the work_day with the given date.

        If an error occurs while creating `work_day`,
        the previously created `work_day` record will be removed.

    Parameters:
        IN in_work_week_id
            ID of the work_week in which the work day should be created.

        IN in_date
            Date of the work day.

        OUT out_work_day_id
            Returns the ID of the created `work_day` record.
            Returns NULL if the work day was not created.

    Errors:
        - 'work_week is not exists.'
            Raised if the provided work_week_id does not exist.

        - 'work_day date is not on work_week.'
            Raised if the provided date does not belong to the specified week.

        - 'Error while creating work_day.'
            Raised if an SQL error occurs during insertion.
*/
DELIMITER $
CREATE PROCEDURE p_create_work_day(
     IN in_work_week_id INT UNSIGNED,
     IN in_date DATE,
     OUT out_work_day_id INT UNSIGNED
)
b_create_work_day: BEGIN
        DECLARE v_week_start_date DATE;
        
        /* Default return value */
        SET out_work_day_id = NULL;

        /* 
           Check that the work_week exists
           and retrieve the start date of the week
        */
        BEGIN
            DECLARE EXIT HANDLER
            FOR NOT FOUND
            SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'work_week is not exist.'
            ;

            SELECT work_week.date
            INTO v_week_start_date
            FROM work_week
            WHERE work_week.id = in_work_week_id
            ;
        END;

        /*
           Validate that the given date belongs to the selected week.
           The date must be between week_start_date and week_start_date + 6 days.
        */
        IF in_date < v_week_start_date OR
           in_date NOT BETWEEN v_week_start_date AND DATE_ADD(v_week_start_date, INTERVAL 6 DAY) THEN
           SIGNAL SQLSTATE '45000'
           SET MESSAGE_TEXT = 'work_day date is not on work_week.'
           ;
        END IF;

        /* Create work_day */
        INSERT INTO work_day(week_id, date)
        VALUES (in_work_week_id, in_date)
        ;
        
        /* Save generated ID */
        SET out_work_day_id = LAST_INSERT_ID();
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_worker_work_default_day;
/*
    Procedure: p_create_worker_work_default_day

    Description:
        Creates a default work day for a specific worker.

        The procedure:
        1. Verifies that the worker exists.
        2. Checks that the worker does not already have a default work day
           for the specified day of the week.
        3. Creates a default work day using p_create_work_default_day.
        4. Links the created default work day with the worker.

    Parameters:
        IN in_day_number
            Number of the day in the week (for example 1–7).

        IN in_worker_id
            ID of the worker for whom the default work day will be created.

    Errors:
        - 'worker does not exist.'
            Raised if the specified worker is not found.

        - 'worker_work_default_day already exists for this worker in this day.'
            Raised if the worker already has a default work day
            for the given day of the week.
*/

DELIMITER $
CREATE PROCEDURE p_create_worker_work_default_day (
     IN in_day_number TINYINT UNSIGNED,
     IN in_worker_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        /*
            Check that the worker exists.
            If no record is found, raise an error.
        */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'worker do not exist.'
                ;

                SELECT id
                INTO v_check
                FROM worker
                WHERE id = in_worker_id
                ;
        END;

        /*
            Check if a default work day already exists
            for this worker on the specified day of the week.
        */
        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of default work day in same day for same worker */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, 
                     work_default_day AS wdd, 
                     worker_work_default_day AS wwdd
                WHERE wwdd.work_default_day_id = wd.id
                AND wdd.work_day_id = wd.id
                AND wwdd.worker_id = in_worker_id
                AND wdd.day_number = in_day_number
                ;

                /* Prevent duplicate default days for the same worker */
                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'Worker already has a default work day for this day.'
                   ;
                END IF;
        END;

        /* Create default work day */
        CALL p_create_work_default_day(in_day_number, v_work_day_id);

        /* Link created default work day to the worker */
        INSERT INTO worker_work_default_day(work_default_day_id, worker_id)
        VALUES (v_work_day_id, in_worker_id)
        ;
END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_create_line_work_default_day;
/*
    Procedure: p_create_line_work_default_day

    Description:
        Creates a default work day for a specific production line.

        The procedure:
        1. Verifies that the line exists.
        2. Checks that the line does not already have a default work day
           for the specified day of the week.
        3. Creates a default work day using p_create_work_default_day.
        4. Links the created default work day with the line.

    Parameters:
        IN in_day_number
            Number of the day in the week (for example 1–7).

        IN in_line_id
            ID of the line for which the default work day will be created.

    Errors:
        - 'line does not exist.'
            Raised if the specified line is not found.

        - 'line_work_default_day already exists for this line in this day.'
            Raised if the line already has a default work day
            for the given day of the week.
*/
DELIMITER $
CREATE PROCEDURE p_create_line_work_default_day (
     IN in_day_number TINYINT UNSIGNED,
     IN in_line_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        /*
            Check that the line exists.
            If no record is found, raise an error.
        */
        BEGIN
                DECLARE EXIT HANDLER
                FOR NOT FOUND
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'worker do not exist.'
                ;

                SELECT id
                INTO v_check
                FROM line
                WHERE id = in_line_id
                ;
        END;

        /*
            Check if a default work day already exists
            for this line on the specified day of the week.
        */
        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd, work_default_day AS wdd, line_work_default_day AS lwdd
                WHERE lwdd.work_default_day_id = wd.id
                AND wdd.work_day_id = wd.id
                AND lwdd.line_id = in_line_id
                AND wdd.day_number = in_day_number
                ;

                /* Prevent duplicate default days for the same line */
                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'line_work_default_day already exists for this line in this day.'
                   ;
                END IF;
        END;

        /* Create default work day */
        CALL p_create_work_default_day(in_day_number, v_work_day_id);

        /* Link created default work day to the line */
        INSERT INTO line_work_default_day(work_default_day_id, line_id)
        VALUES (v_work_day_id, in_line_id)
        ;
END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_create_worker_work_day;
/*
    Procedure: p_create_worker_work_day

    Description:
        Creates a work day record for a worker and assigns the worker
        to a specific production line and role for that day.

        The procedure performs several validation steps before creating
        the assignment:

        1. Verifies that the worker exists.
        2. Verifies that the specified line_work_day exists and retrieves
           the corresponding line_id, work_week_id, and date.
        3. Checks whether the worker is already assigned to the same
           line_work_day for the specified role.
        4. Creates a worker-specific work_day using p_create_work_day.
        5. Inserts a record into worker_work_day linking the worker,
           the line, the role, and the created work_day.

        The created work_day_id is returned through the OUT parameter.

    Parameters:
        IN in_line_work_day_id
            Identifier of the line work day.

        IN in_worker_id
            Identifier of the worker to assign.

        IN in_role_id
            Identifier of the role the worker will perform.

        OUT out_worker_work_day_id
            Identifier of the created worker work_day.

    Returns:
        None (uses OUT parameter).

    Errors:
        SQLSTATE '45000' is raised when:
            - The worker does not exist.
            - The specified line_work_day does not exist.
            - The worker is already assigned to the same line_work_day
              for the same role.

    Notes:
        - The procedure relies on p_create_work_day to create a
          worker-specific work day within the correct work week.

        - Each worker receives their own work_day record even when
          multiple workers work on the same production line day.
*/
DELIMITER $
CREATE PROCEDURE p_create_worker_work_day (
    IN in_line_work_day_id INT UNSIGNED,
    IN in_worker_id INT UNSIGNED,
    IN in_role_id INT UNSIGNED,
    OUT out_worker_work_day_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_line_id INT UNSIGNED;
        DECLARE v_work_week_id INT UNSIGNED;
        DECLARE v_date DATE;
        DECLARE v_work_day_id INT UNSIGNED;

        /*
            Check that the worker exists.
        */
        BEGIN
            DECLARE EXIT HANDLER
            FOR NOT FOUND
            SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Worker does not exist.';

            SELECT id
            INTO v_check
            FROM worker
            WHERE id = in_worker_id;
        END;

        /*
            Check that the line_work_day exists and get line_id, week_id, date
        */
        BEGIN
            DECLARE EXIT HANDLER
            FOR NOT FOUND
            SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Line_work_day does not exist.';

            SELECT line_work_day.line_id, work_day.week_id, work_day.date
            INTO v_line_id, v_work_week_id, v_date
            FROM line_work_day, work_day
            WHERE line_work_day.work_day_id = in_line_work_day_id
            AND line_work_day.work_day_id = work_day.id;
        END;

        /*
            Check if the worker is already assigned to this line_work_day
            for any role
        */
        BEGIN
            DECLARE CONTINUE HANDLER
            FOR NOT FOUND
            SET v_check = NULL;

            SELECT worker_id
            INTO v_check
            FROM worker_work_day
            WHERE line_work_day_id = in_line_work_day_id
            AND worker_id = in_worker_id
            AND role_id = in_role_id;

            IF v_check IS NOT NULL THEN
                SIGNAL SQLSTATE '45000'
                SET MESSAGE_TEXT = 'Worker is already assigned to this line_work_day in this role.';
            END IF;
        END;

        /* Create work week day */
        CALL p_create_work_day(v_work_week_id, v_date, v_work_day_id);

        /*
            Assign the worker to the line_work_day
        */
        INSERT INTO worker_work_day(work_day_id, line_work_day_id, line_id, worker_id, role_id)
        VALUES (v_work_day_id, in_line_work_day_id, v_line_id, in_worker_id, in_role_id);

        SET out_worker_work_day_id = v_work_day_id;
END;
$
DELIMITER ;



DROP PROCEDURE IF EXISTS p_create_line_work_day;

/*
    Procedure: p_create_line_work_day

    Description:
        Creates a scheduled work day for a production line within a specific work week.

        The procedure:
        1. Verifies that the line exists.
        2. Checks that the line does not already have a work day
           for the specified date within the given week.
        3. Creates the work week day using p_create_work_day.
        4. Links the created work day with the specified line.

    Parameters:
        IN in_work_week_id
            ID of the work week in which the work day will be created.

        IN in_date
            Date of the work day.

        IN in_line_id
            ID of the production line assigned to the work day.

    Errors:
        - 'line does not exist.'
            Raised if the specified line is not found.

        - 'line_work_day already exists for this line in this date.'
            Raised if the line already has a work assignment
            for the specified date in the selected week.
*/
DELIMITER $
CREATE PROCEDURE p_create_line_work_day (
     IN in_work_week_id INT UNSIGNED,
     IN in_date DATE,
     IN in_line_id INT UNSIGNED
)
BEGIN
        DECLARE v_check INT UNSIGNED;
        DECLARE v_work_day_id INT UNSIGNED;

        /*
            Check that the line exists.
            If the line is not found, raise an error.
        */
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

        /*
            Check if a work assignment already exists
            for the same line and date within the selected week.
        */
        BEGIN
                DECLARE CONTINUE HANDLER
                FOR NOT FOUND
                SET v_check = NULL;
                
                /* check on existing of week work day in same date for same line */
                SELECT wd.id
                INTO v_check
                FROM work_day AS wd,
                     line_work_day AS lwd
                WHERE lwd.work_day_id = wd.id
                AND wd.week_id = in_work_week_id
                AND lwd.line_id = in_line_id
                AND wd.date = in_date
                ;

                IF v_check IS NOT NULL THEN
                   SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'line_work_day already exists for this line in this date.'
                   ;
                END IF;
        END;

        /* Create work week day */
        CALL p_create_work_day(in_work_week_id, in_date, v_work_day_id);

        /* Link created work day to the line */
        INSERT INTO line_work_day(work_day_id, line_id)
        VALUES (v_work_day_id, in_line_id)
        ;
END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_insert_work_time;
/*
    Procedure: p_insert_work_time

    Description:
        Inserts a work time interval for a given work_day.

        Before inserting, the procedure checks for existing
        time intervals that overlap with the provided interval.
        If overlapping intervals are found, they are merged into
        a single continuous interval.

        The procedure performs the following steps:

        1. Validates the input interval.
        2. Searches for existing intervals whose range overlaps
           the provided start_time.
        3. Searches for existing intervals whose range overlaps
           the provided end_time.
        4. Expands the interval boundaries if overlapping intervals
           are found.
        5. Deletes all intervals fully contained inside the new
           merged interval.
        6. Inserts the resulting merged interval.

        This ensures that the table never contains multiple
        overlapping time ranges for the same work_day.

    Parameters:
        IN in_work_day_id
            Identifier of the work_day to which the time interval belongs.

        IN in_start_time
            Start time of the work interval.

        IN in_end_time
            End time of the work interval.

    Notes:
        - If the input interval is invalid (NULL values or end_time
          earlier than start_time), the procedure exits without
          modifying the database.

        - Overlapping intervals are automatically merged to maintain
          a normalized schedule representation.

        - IMPORTANT (time boundary behavior):
          The procedure treats intervals as continuous ranges and
          merges them when they overlap. If two intervals touch
          exactly at their boundaries (for example 08:00–10:00 and
          10:00–12:00), the result may depend on how the BETWEEN
          condition is evaluated.

          When precise control over schedule boundaries is required,
          it is recommended to generate work schedules using smaller
          time chunks and avoid relying on edge-aligned intervals.
*/
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

    BEGIN
        DECLARE v_check INT UNSIGNED;

        DECLARE EXIT HANDLER
        FOR NOT FOUND
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'work_day is not exist.';

        SELECT work_day.id
        INTO v_check
        FROM work_day
        WHERE work_day.id = in_work_day_id
        ;

    END;

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

    /* remove intervals fully contained in the merged range */
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


DROP PROCEDURE IF EXISTS p_remove_work_time;
/*
    Procedure: p_remove_work_time

    Description:
        Removes a specified time interval from a work day schedule.
        The procedure updates the `work_time` table so that the
        interval [in_start_time, in_end_time] is excluded from the
        work day.

        Depending on how existing intervals overlap with the
        removal interval, the procedure will:
            - delete intervals fully inside the range,
            - trim intervals partially overlapping the range,
            - split intervals that completely contain the range.

    Parameters:
        IN in_work_day_id
            Identifier of the work day whose schedule will be modified.

        IN in_start_time
            Start of the interval to remove.

        IN in_end_time
            End of the interval to remove.

    Notes:
        - The procedure assumes that intervals in `work_time`
          do not overlap initially.
        - After execution, the resulting schedule will contain
          no time segments inside the removed interval.
        - If the removal interval splits an existing work period,
          the original interval will be divided into two parts.
*/
DELIMITER $
CREATE PROCEDURE p_remove_work_time(
    IN in_work_day_id INT UNSIGNED,
    IN in_start_time TIME,
    IN in_end_time TIME
)
BEGIN
    DECLARE v_continue BOOLEAN DEFAULT TRUE;
    DECLARE v_work_day_id INT UNSIGNED;
    DECLARE v_work_time_id INT UNSIGNED;
    DECLARE v_end_time TIME;
    
    /* =========================================================
    VALIDATION: PREVENT REMOVING LINE WORK TIME USED BY WORKERS
    ---------------------------------------------------------
    Check whether any worker already has assigned work time
    within the interval that is about to be removed from the
    line schedule.

    If at least one worker_work_day has a work_time interval
    that overlaps with [in_start_time, in_end_time], the
    operation must be aborted to avoid breaking consistency
    between line schedules and worker schedules.

    Overlap cases checked:
        - worker interval starts inside the removal range
        - worker interval ends inside the removal range
        - worker interval fully contains the removal range

    If such an overlap exists, an error is raised and the
    procedure stops execution.
    ========================================================= */
    IF EXISTS (SELECT 1
        FROM worker_work_day, work_time
        WHERE worker_work_day.line_work_day_id = in_work_day_id
        AND worker_work_day.work_day_id = work_time.work_day_id
        AND (
            (in_start_time < start_time AND start_time < in_end_time) OR
            (in_start_time < end_time AND end_time < in_end_time) OR
            (start_time < in_start_time AND in_end_time < end_time)
        )) THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Cannot use p_remove_work_time to remove work time from line_work_day with workers in this time'
        ;
    END IF;

    /* Delete intervals fully inside the removal range */
    DELETE FROM work_time
    WHERE work_day_id = in_work_day_id
    AND (start_time BETWEEN in_start_time AND in_end_time)
    AND (end_time BETWEEN in_start_time AND in_end_time)
    ;

    /* Trim intervals starting inside the range */
    UPDATE work_time
    SET start_time = in_end_time
    WHERE work_day_id = in_work_day_id
    AND (start_time BETWEEN in_start_time AND in_end_time)
    AND NOT (end_time BETWEEN in_start_time AND in_end_time)
    ;

    /* Trim intervals ending inside the range */
    UPDATE work_time
    SET end_time = in_start_time
    WHERE work_day_id = in_work_day_id
    AND NOT (start_time BETWEEN in_start_time AND in_end_time)
    AND (end_time BETWEEN in_start_time AND in_end_time)
    ;

    BEGIN

        DECLARE CONTINUE HANDLER
        FOR NOT FOUND
        SET v_continue = FALSE
        ;

        /* Find interval that fully contains the removal range */
        SELECT id, work_day_id, end_time
        INTO v_work_time_id, v_work_day_id, v_end_time
        FROM work_time
        WHERE work_day_id = in_work_day_id
        AND (in_start_time BETWEEN start_time AND end_time)
        AND (in_end_time BETWEEN start_time AND end_time)
        ; 

         /* Split the interval into two parts */
        IF v_continue = TRUE THEN

            UPDATE work_time
            SET end_time = in_start_time
            WHERE work_time.id = v_work_time_id
            ;

            INSERT INTO work_time(work_day_id, start_time, end_time)
            VALUES (v_work_day_id, in_end_time, v_end_time)
            ;
        END IF;

    END;
END;
$
DELIMITER ;


DROP PROCEDURE p_remove_line_work_day_work_time;
/*
    Procedure: p_remove_line_work_day_work_time

    Description:
        Removes a time interval from a line work day schedule and
        synchronizes the change with all workers assigned to that line.

    Parameters:
        IN in_line_work_day_id
            Identifier of the line_work_day whose schedule will be modified.

        IN in_start_time
            Start of the interval to remove.

        IN in_end_time
            End of the interval to remove.

    Notes:
        - Worker schedules are updated first to maintain consistency
          between worker schedules and the line schedule.
        - The actual interval removal logic is implemented in
          p_remove_work_time.
        - If the provided ID does not belong to a line_work_day,
          the procedure stops with an error.
*/

DELIMITER $
CREATE PROCEDURE p_remove_line_work_day_work_time(
    IN in_line_work_day_id INT UNSIGNED,
    IN in_start_time TIME,
    IN in_end_time TIME
)
BEGIN
    DECLARE v_worker_work_day_id INT UNSIGNED;
    DECLARE v_continue BOOLEAN DEFAULT TRUE;

    /* =========================================================
       CURSOR: FETCH ALL WORKER WORK DAYS ASSIGNED TO THE LINE
       ========================================================= */
    DECLARE c_worker_work_days CURSOR FOR
    SELECT work_day_id
    FROM worker_work_day
    WHERE line_work_day_id = in_line_work_day_id
    ;

    DECLARE CONTINUE HANDLER
    FOR NOT FOUND
    SET v_continue = FALSE;

    /* VALIDATION: PREVENT REMOVING WORK TIME NOT FROM LINE SCHEDULE */
    IF NOT EXISTS (SELECT 1
        FROM line_work_day
        WHERE work_day_id = in_line_work_day_id) THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Cannot use p_remove_work_time to remove work time not from line_work_day'
        ;
    END IF;

    OPEN c_worker_work_days;

    /* =========================================================
       LOOP THROUGH ALL WORKERS ASSIGNED TO THE LINE
       ========================================================= */
    b_worker_work_days: LOOP

        FETCH c_worker_work_days
        INTO v_worker_work_day_id
        ;

        IF v_continue = FALSE THEN
            LEAVE b_worker_work_days;
        END IF;

        /* Remove the interval from the worker schedule */
        CALL p_remove_work_time(v_worker_work_day_id, in_start_time, in_end_time);

    END LOOP b_worker_work_days;

    CLOSE c_worker_work_days;

    /* =========================================================
       REMOVE THE INTERVAL FROM THE LINE SCHEDULE ITSELF
       ========================================================= */
    CALL p_remove_work_time(in_line_work_day_id, in_start_time, in_end_time); 

END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_generate_line_work_time_chunk;
/*
    Procedure: p_generate_line_work_time_chunk

    Description:
        Generates and assigns work time chunks for a specific
        production line, date, and role.

        The procedure automatically builds a work schedule
        based on the default work configuration of the line
        and worker availability.

        Main workflow:

        1. Ensure that the corresponding work_week exists
           for the provided date. If not, it is created.

        2. Retrieve the default work time interval configured
           for the production line.

        3. Ensure that a line_work_day record exists for the
           specified line and date.

        4. Split the default work interval into smaller
           time chunks using all known time boundaries.

        5. Determine which workers:
           - have the required role
           - are scheduled to work that weekday
           - are currently available
           and place them into a temporary queue.

        6. Calculate the total available work time for each worker
           to prioritize workers who can cover larger intervals.

        7. Assign workers to time chunks while respecting the
           maximum number of workers allowed for each role.

        8. Insert work_time intervals both for the worker
           and for the production line.

    Parameters:
        IN in_date
            Date for which the schedule is generated.

        IN in_id_line
            Identifier of the production line.

        IN in_work_time
            Identifier of the default work_time interval.

        IN in_role_id
            Identifier of the worker role to assign.

    Notes:

        IMPORTANT:
        The scheduling algorithm operates on small time chunks
        derived from the boundaries of default work intervals.
        This approach avoids inaccuracies that may occur when
        assigning workers to large time intervals where workers
        may only partially overlap.

        Example problem avoided by chunking:

            Required workers: 2

            Worker A: 08:00–10:00
            Worker B: 10:00–12:00

            If scheduling was performed on interval 08:00–12:00,
            the system could incorrectly assume that two workers
            are available simultaneously.

            By splitting the interval into chunks, the system
            ensures correct worker allocation.
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
        DECLARE v_line_work_day_id INT UNSIGNED;
        DECLARE v_start_time TIME;
        DECLARE v_end_time TIME;
        
        /* =========================================================
           Ensure work_week exists for the given date.
           If no week is found, a new one is created.
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
           Retrieve the default work interval for the line.
           If no configuration exists the procedure stops.
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
           Ensure line_work_day exists for the line and date.
           If not found it will be created.
           ========================================================= */
        BEGIN
            /* Create if not exists */
            DECLARE CONTINUE HANDLER
            FOR NOT FOUND
            BEGIN
                CALL p_create_line_work_day(v_work_week_id, in_date, in_id_line);
                SET v_line_work_day_id = LAST_INSERT_ID();
            END;
            
            /* Try to find existing record */
            SELECT lwd.work_day_id
            INTO v_line_work_day_id
            FROM line_work_day AS lwd,
                 work_day AS wd
            WHERE lwd.line_id = in_id_line
            AND wd.id = lwd.work_day_id
            AND wd.date = in_date
            ;
        END;

        /* =========================================================
            Temporary structures used during scheduling.
            
            workers_queue
                stores candidate workers for each time chunk

            workers_queue_number
                stores total available work time for each worker
                (used to prioritize assignment)
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
           Split default interval into smaller chunks using
           all time boundaries found in the configuration.

           Each chunk represents the smallest possible
           assignable scheduling interval.
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

                /* Insert workers into queue who:
                    - have required role
                    - are available in this chunk
                */
                INSERT INTO workers_queue(worker_id, start_time, end_time)
                SELECT wwdd.worker_id, v_prev_time, v_current_time
                FROM worker_work_default_day AS wwdd,
                        work_default_day AS wdd,
                        worker_role AS wr
                WHERE wwdd.work_default_day_id = wdd.work_day_id
                AND wdd.day_number = WEEKDAY(in_date)
                AND wr.worker_id = wwdd.worker_id
                AND wr.role_id = in_role_id
                AND f_can_work(wwdd.worker_id, in_date, v_prev_time, v_current_time) = TRUE
                ;

                    SET v_prev_time = v_current_time;

            END LOOP l_time;

            CLOSE c_time;
        END;

        /* =========================================================
           Calculate total available time per worker.

           Workers with longer availability will be prioritized
           during assignment.
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
            DECLARE v_worker_work_day_id INT UNSIGNED;
            DECLARE v_continue BOOLEAN DEFAULT TRUE;

            DECLARE CONTINUE HANDLER
            FOR NOT FOUND
            SET v_continue = FALSE
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
                AND end_time = v_end_time
                ;

                /* Skip if already full */
                IF f_is_enough_workers(v_line_work_day_id, in_role_id, v_start_time, v_end_time) THEN
                    SELECT CONCAT('Chunk ',
                                    CAST(v_start_time AS CHAR),
                                    ' -> ',
                                    CAST(v_end_time AS CHAR),
                                    ' for line ',
                                    CAST(in_id_line AS CHAR),
                                    ' role: ',
                                    CAST(in_role_id AS CHAR),
                                    ' already full') AS message;

                    ITERATE l_workers_queue;
                END IF;

                /* Ensure worker_work_day exists */
                BEGIN

                    DECLARE CONTINUE HANDLER
                    FOR NOT FOUND
                    BEGIN
                        CALL p_create_worker_work_day(
                                v_line_work_day_id,
                                v_worker_id,
                                in_role_id,
                                v_worker_work_day_id
                        );
                    END;

                    SELECT wwd.work_day_id
                    INTO v_worker_work_day_id
                    FROM worker_work_day AS wwd,
                        work_day AS wd
                    WHERE wwd.line_work_day_id = v_line_work_day_id
                    AND wd.id = wwd.work_day_id
                    AND wwd.worker_id = v_worker_id
                    AND wwd.role_id = in_role_id
                    ;

                END;

                /* Insert time for worker and for line */
                CALL p_insert_work_time(
                        v_worker_work_day_id,
                        v_start_time,
                        v_end_time
                );
                CALL p_insert_work_time(
                        v_line_work_day_id,
                        v_start_time,
                        v_end_time
                );

            END LOOP l_workers_queue;
        END;
END;
$
DELIMITER ;


DROP PROCEDURE IF EXISTS p_generate_line_work_time;
/*
    Procedure: p_generate_line_work_time

    Description:
        Generates a full work schedule for a production line on a given date
        based on the roles required for that line.

        The procedure iterates through all roles configured for the line
        in the `line_role` table. For each role it determines how many
        workers are required (`worker_count`) and repeatedly calls
        `p_generate_line_work_time_chunk` to assign workers to the
        generated time chunks.

        This procedure acts as a high-level coordinator that triggers
        the scheduling process for every role required on the line.

    Parameters:
        IN in_line_id
            Identifier of the production line.

        IN in_work_time_id
            Identifier of the default work_time interval that defines
            the working period for the line.

        IN in_date
            Date for which the schedule should be generated.

    Notes:
        - Roles and required worker counts are retrieved from
          the `line_role` table.

        - For each role the procedure calls
          `p_generate_line_work_time_chunk` multiple times according
          to the required number of workers.

        - The actual worker assignment logic is implemented in
          `p_generate_line_work_time_chunk`.

        - IMPORTANT:
          The loop uses `worker_count` as the number of scheduling
          iterations for the role. Each iteration attempts to assign
          one additional worker for the generated time chunks.
*/
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
        SELECT role_id, worker_count
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
/*
    Procedure: p_generate_day_work_time

    Description:
        Generates a complete schedule for all production lines for a given date.
        The procedure iterates through all lines that have default work times 
        for the weekday corresponding to `in_date`. For each line, it calls 
        `p_generate_line_work_time` to create role-based schedules and assign workers.

    Algorithm:
        1. Determine the weekday of `in_date`.
        2. Fetch all lines with default work times for that weekday from 
           `line_work_default_day` joined with `work_default_day` and `work_time`.
        3. Sort the lines by total default time (longest intervals first).
        4. For each line and default time interval:
            a. Call `p_generate_line_work_time` to assign workers to roles
               and generate work time chunks.
        5. Repeat until all lines for the date are processed.

    Parameters:
        IN in_date
            Date for which the schedule should be generated.

    Notes:
        - Lines are ordered by the length of the default work interval
          (TIMEDIFF(end_time, start_time)) to prioritize longer shifts first.
        - Worker assignment, role validation, and time chunk generation
          are handled inside the `p_generate_line_work_time` and
          `p_generate_line_work_time_chunk` procedures.
        - Ensure that default work times exist for all lines; otherwise,
          those lines will not be scheduled.
*/
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