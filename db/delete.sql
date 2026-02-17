
DROP TRIGGER IF EXISTS trg_delete_wd_after_wwwd_delete;
DROP TRIGGER IF EXISTS trg_delete_wd_after_wwdd_delete;

DROP TABLE IF EXISTS line_work_week_day;
DROP TABLE IF EXISTS worker_work_week_day;

DROP TABLE IF EXISTS line_work_default_day;
DROP TABLE IF EXISTS worker_work_default_day;

DROP TABLE IF EXISTS work_week_day;
DROP TABLE IF EXISTS work_default_day;

DROP TABLE IF EXISTS work_time;
DROP TABLE IF EXISTS work_day;
DROP TABLE IF EXISTS line_role;
DROP TABLE IF EXISTS worker_role;

DROP TABLE IF EXISTS work_week;
DROP TABLE IF EXISTS line;
DROP TABLE IF EXISTS role;
DROP TABLE IF EXISTS worker;
