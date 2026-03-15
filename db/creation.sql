


CREATE TABLE worker(
       id INT UNSIGNED AUTO_INCREMENT,
       first_name VARCHAR(100) NOT NULL,
       second_name VARCHAR(100) NOT NULL,
       PRIMARY KEY (id)
);

CREATE TABLE role(
       id INT UNSIGNED AUTO_INCREMENT,
       name VARCHAR(255) NOT NULL,
       PRIMARY KEY (id),
       CONSTRAINT unq_role_name UNIQUE (name)
);

CREATE TABLE line(
       id INT UNSIGNED AUTO_INCREMENT,
       name VARCHAR(100) NOT NULL,
       product VARCHAR(255) NOT NULL,
       PRIMARY KEY (id),
       CONSTRAINT unq_name UNIQUE (name)
);

CREATE TABLE work_week(
       id INT UNSIGNED AUTO_INCREMENT,
       date DATE NOT NULL,
       CONSTRAINT chk_date CHECK (WEEKDAY(date) = 0),
       PRIMARY KEY (id),
       CONSTRAINT ww_unq_date UNIQUE (date)
);

CREATE TABLE worker_role(
       worker_id INT UNSIGNED NOT NULL,
       role_id INT UNSIGNED NOT NULL,
       PRIMARY KEY (worker_id, role_id),
       CONSTRAINT wr_fk_worker_id FOREIGN KEY (worker_id) REFERENCES worker(id)
       ON DELETE CASCADE,
       CONSTRAINT wr_fk_role_id FOREIGN KEY (role_id) REFERENCES role(id)
       ON DELETE CASCADE
);

CREATE TABLE line_role(
       line_id INT UNSIGNED NOT NULL,
       role_id INT UNSIGNED NOT NULL,
       worker_count SMALLINT NOT NULL,
       PRIMARY KEY (line_id, role_id),
       CONSTRAINT lr_fk_line_id FOREIGN KEY (line_id) REFERENCES line(id)
       ON DELETE CASCADE,
       CONSTRAINT lr_fk_role_id FOREIGN KEY (role_id) REFERENCES role(id)
);

CREATE TABLE work_day(
       id INT UNSIGNED AUTO_INCREMENT,
       week_id INT UNSIGNED,
       date DATE,
       PRIMARY KEY (id),
       CONSTRAINT wd_fk_week_id FOREIGN KEY (week_id) REFERENCES work_week(id)
       ON DELETE CASCADE
);

CREATE TABLE work_time(
       id INT UNSIGNED AUTO_INCREMENT,
       work_day_id INT UNSIGNED NOT NULL,
       start_time TIME NOT NULL,
       end_time TIME NOT NULL,
       PRIMARY KEY (id),
       CONSTRAINT chk_start_end_time CHECK (start_time < end_time),
       CONSTRAINT wt_fk_work_day_id FOREIGN KEY (work_day_id) REFERENCES work_day(id)
       ON DELETE CASCADE
);

CREATE TABLE work_default_day(
       work_day_id INT UNSIGNED NOT NULL,
       day_number TINYINT UNSIGNED NOT NULL,
       PRIMARY KEY (work_day_id),
       CONSTRAINT chk_day_number CHECK (day_number BETWEEN 0 AND 6), /* 0 = Monday, 6 = Sunday */
       CONSTRAINT wdd_fk_work_day_id FOREIGN KEY (work_day_id) REFERENCES work_day(id)
       ON DELETE CASCADE
);

CREATE TABLE worker_work_default_day(
       work_default_day_id INT UNSIGNED NOT NULL,
       worker_id INT UNSIGNED NOT NULL,
       PRIMARY KEY (work_default_day_id, worker_id),
       CONSTRAINT wwdd_fk_work_default_day_id
       FOREIGN KEY (work_default_day_id) REFERENCES work_default_day(work_day_id)
       ON DELETE CASCADE,
       CONSTRAINT wwdd_fk_worker_id FOREIGN KEY (worker_id) REFERENCES worker(id)
       ON DELETE CASCADE
);

CREATE TABLE line_work_default_day(
       work_default_day_id INT UNSIGNED NOT NULL,
       line_id INT UNSIGNED NOT NULL,
       PRIMARY KEY (work_default_day_id, line_id),
       CONSTRAINT lwdd_fk_work_default_day_id FOREIGN KEY (work_default_day_id) REFERENCES work_default_day(work_day_id)
       ON DELETE CASCADE,
       CONSTRAINT lwdd_fk_line_id FOREIGN KEY (line_id) REFERENCES line(id)
);

CREATE TABLE line_work_day(
       work_day_id INT UNSIGNED NOT NULL,
       line_id INT UNSIGNED NOT NULL,
       PRIMARY KEY (work_day_id, line_id),
       CONSTRAINT lwd_fk_work_day_id
       FOREIGN KEY (work_day_id) REFERENCES work_day(id)
       ON DELETE CASCADE,
       CONSTRAINT lwd_fk_line_id FOREIGN KEY (line_id) REFERENCES line(id)
       ON DELETE CASCADE
);

CREATE TABLE worker_work_day(
       work_day_id INT UNSIGNED NOT NULL,
       line_work_day_id INT UNSIGNED NOT NULL,
       line_id INT UNSIGNED NOT NULL,
       worker_id INT UNSIGNED NOT NULL,
       role_id INT UNSIGNED NOT NULL,
       PRIMARY KEY (work_day_id, line_work_day_id, worker_id, role_id),
       CONSTRAINT wwd_fk_work_day_id
       FOREIGN KEY (work_day_id) REFERENCES work_day(id),
       CONSTRAINT wwd_fk_line_work_day_id
       FOREIGN KEY (line_work_day_id, line_id) REFERENCES line_work_day(work_day_id, line_id)
       ON DELETE CASCADE,
       CONSTRAINT wwd_fk_worker_id FOREIGN KEY (worker_id) REFERENCES worker(id)
       ON DELETE CASCADE,
       CONSTRAINT wwd_fk_role_id FOREIGN KEY (role_id) REFERENCES role(id)
       ON DELETE CASCADE
);