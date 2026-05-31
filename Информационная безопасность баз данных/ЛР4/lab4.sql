DROP TABLE IF EXISTS departments CASCADE;
DROP TABLE IF EXISTS employees;

DROP POLICY IF EXISTS user_row_policy ON employees;
DROP TABLE IF EXISTS logs;
DROP SCHEMA IF EXISTS subschema_user1 CASCADE;
DROP SCHEMA IF EXISTS subschema_user2 CASCADE;




CREATE USER user1 WITH PASSWORD 'passwd1';
CREATE USER user2 WITH PASSWORD 'passwd2';
CREATE USER user3 WITH PASSWORD 'passwd3';
CREATE USER user4 WITH PASSWORD 'passwd4';


/* 1. */
CREATE TABLE departments (
    dept_id SERIAL PRIMARY KEY,
    dept_name TEXT NOT NULL
);

CREATE TABLE employees (
    emp_id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    salary NUMERIC,
    dept_id INTEGER REFERENCES departments(dept_id),
    created_by TEXT DEFAULT current_user
);

INSERT INTO departments (dept_name) VALUES ('IT'), ('Sales'), ('HR');
INSERT INTO employees (name, salary, dept_id) VALUES 
('Ivanov', 50000, 1),
('Petrov', 60000, 1),
('Sidorov', 45000, 2),
('Kuznetsov', 55000, 3);


/* 2. */
GRANT ALL PRIVILEGES ON TABLE departments TO user1;
GRANT ALL PRIVILEGES ON TABLE employees TO user1;
GRANT INSERT, SELECT, UPDATE ON TABLE departments TO user2;
GRANT INSERT, SELECT, UPDATE ON TABLE employees TO user2;
GRANT DELETE ON TABLE departments TO user3 WITH GRANT OPTION;
GRANT DELETE ON TABLE employees TO user3 WITH GRANT OPTION;

/* 3. */
SET ROLE user3;
GRANT DELETE ON TABLE departments TO user4;
GRANT DELETE ON TABLE employees TO user4;
RESET ROLE;


/* 4. */
REVOKE ALL PRIVILEGES ON TABLE departments, employees FROM user1;
REVOKE INSERT, SELECT, UPDATE ON TABLE departments, employees FROM user2;
REVOKE DELETE ON TABLE employees, departments FROM user3 CASCADE;

-- \dp employees
-- \dp departments

-- /* 5. */
CREATE SCHEMA subschema_user1 AUTHORIZATION user1;
CREATE SCHEMA subschema_user2 AUTHORIZATION user2;

SET ROLE user1;
CREATE TABLE subschema_user1.work_tasks (
    task_id SERIAL PRIMARY KEY,
    description TEXT,
    deadline DATE
);
RESET ROLE;

SET ROLE user2;
CREATE TABLE subschema_user2.sales_reports (
    report_id SERIAL PRIMARY KEY,
    amount NUMERIC,
    region TEXT
);
RESET ROLE;

-- SET ROLE user1;
-- SELECT * FROM subschema_user2.sales_reports;
-- RESET ROLE;

/* 6. */
CREATE VIEW employees_info AS
SELECT e.name, d.dept_name
FROM employees AS e
JOIN departments AS d ON e.dept_id = d.dept_id;

GRANT SELECT ON employees_info TO user2;


-- SET ROLE user2;
-- SELECT * FROM employees_info;
-- RESET ROLE;


/* 7. */

-- GRANT SELECT, INSERT ON employees TO user1, user2;
-- GRANT USAGE, SELECT ON SEQUENCE employees_emp_id_seq TO user1, user2;
-- SET ROLE user1;
-- INSERT INTO employees (name, salary, dept_id, created_by)
-- VALUES ('Pakhomov', 100000, 1, 'user1');
-- RESET ROLE;
-- SET ROLE user2;
-- INSERT INTO employees (name, salary, dept_id, created_by)
-- VALUES ('Brown', 90000, 2, 'user2');
-- RESET ROLE;

-- \dp employees

-- ALTER TABLE employees ENABLE ROW LEVEL SECURITY;

-- CREATE POLICY user_row_policy
-- ON employees
-- FOR SELECT
-- TO public
-- USING (created_by = current_user);

-- SELECT * FROM employees;

-- SET ROLE user1;
-- SELECT * FROM employees;
-- RESET ROLE;

-- SET ROLE user2;
-- SELECT * FROM employees;
-- RESET ROLE;

-- \dp employees


-- /* 8. */
CREATE TABLE logs (
    log_id SERIAL PRIMARY KEY,
    table_name TEXT,
    operation TEXT,
    user_name TEXT,
    old_data JSONB,
    new_data JSONB,
    changed_at TIMESTAMP DEFAULT now()
);
-- CREATE POLICY user_row_policy ON employees 
--     FOR ALL -- Даем права на все операции (SELECT, UPDATE, DELETE)
--     TO PUBLIC 
--     USING (created_by = current_user)
--     WITH CHECK (created_by = current_user);


-- GRANT SELECT, INSERT ON logs TO user1, user2;
-- GRANT USAGE, SELECT ON SEQUENCE logs_log_id_seq TO user1, user2;

GRANT SELECT, INSERT, UPDATE, DELETE ON employees TO user1, user2;
GRANT USAGE, SELECT ON SEQUENCE employees_emp_id_seq TO user1, user2;



CREATE OR REPLACE FUNCTION logs_trigger() RETURNS trigger AS $$
BEGIN
    IF (TG_OP = 'INSERT') THEN
        INSERT INTO logs (table_name, operation, user_name, new_data)
        VALUES (TG_TABLE_NAME, TG_OP, current_user, row_to_json(NEW)::jsonb);
    ELSIF (TG_OP = 'UPDATE') THEN
        INSERT INTO logs (table_name, operation, user_name, old_data, new_data)
        VALUES (TG_TABLE_NAME, TG_OP, current_user, row_to_json(OLD)::jsonb, row_to_json(NEW)::jsonb);
    ELSIF (TG_OP = 'DELETE') THEN
        INSERT INTO logs (table_name, operation, user_name, old_data)
        VALUES (TG_TABLE_NAME, TG_OP, current_user, row_to_json(OLD)::jsonb);
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql
SECURITY DEFINER;

CREATE TRIGGER trigger_employees_logs
AFTER INSERT OR UPDATE OR DELETE ON employees
FOR EACH ROW EXECUTE FUNCTION logs_trigger();



SET ROLE user1;
INSERT INTO employees (name, salary, dept_id, created_by) 
VALUES ('AuditTest', 77777, 1, 'user1');
RESET ROLE;



SET ROLE user2;
UPDATE employees SET salary = 95000 WHERE name = 'Kuznetsov';
RESET ROLE;


SELECT * FROM logs;
SELECT * FROM employees;


\dp logs;