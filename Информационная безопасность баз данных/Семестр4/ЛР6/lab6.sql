DROP TABLE IF EXISTS students_info;
DROP TABLE IF EXISTS finances;
DROP TABLE IF EXISTS users;


/* 1. */
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    password VARCHAR(50) NOT NULL
);

CREATE TABLE students_info (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id),
    full_name VARCHAR(100),
    city VARCHAR(50),
    university VARCHAR(50),
    degree VARCHAR(50)
);

CREATE TABLE finances (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id),
    salary_amount INT
);



INSERT INTO users (username, password) VALUES
('admin', 'admin123'),
('petrov_p', 'qwerty789'),
('ivanov_i', 'pass123'),
('sidorov_s', 'secret_key'),
('romanov_r', 'roma99'),
('elena_v', '123456');

INSERT INTO students_info (user_id, full_name, city, university, degree) VALUES
(1, 'Administrator', 'Spb', 'ITMO', 'Master'),
(2, 'Petr Petrov', 'Spb', 'ITMO', 'Ph.D.'),
(3, 'Ivan Ivanov', 'Spb', 'ITMO', NULL),
(4, 'Sidor Sidorov', 'Moscow', 'MSU', 'Bachelor'),
(5, 'Roman Romanov', 'Spb', 'LETI', 'Bachelor'),
(6, 'Elena Vasilyeva', 'Moscow', 'MSU', 'Master');

INSERT INTO finances (user_id, salary_amount) VALUES
(1, 70000),
(2, 45000),
(3, 30000),
(4, 35000),
(5, 32000),
(6, 40000);


SELECT * FROM users;
SELECT * FROM students_info;
SELECT * FROM finances;
