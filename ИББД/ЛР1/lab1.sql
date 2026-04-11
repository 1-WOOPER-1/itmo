DROP DATABASE IF EXISTS n3250_467036;


/* 2 */
CREATE DATABASE n3250_467036;


/* 3 */
\c n3250_467036


/* 4 */
CREATE SCHEMA n3250_467036_schema_lab1;


/* 5 */
CREATE TABLE n3250_467036_tbl1 (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100)
);

CREATE TABLE n3250_467036_schema_lab1.n3250_467036_tbl2 (
    id SERIAL PRIMARY KEY,
    description TEXT
);


/* 6 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_table_i5 (
    num INTEGER,
    name  VARCHAR(30),
    code CHAR(5),
    create_date TIMESTAMP,
    birth_date DATE,
    file_data BYTEA
);


/* 7 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_table_i6 (
    id SERIAL PRIMARY KEY,
    quantity INTEGER DEFAULT 0,
    timeinterval INTERVAL
);


/* 8 */
CREATE TYPE n3250_467036_schema_lab1.n3250_467036_type AS (
    width REAL,
    height REAL,
    produced_date DATE,
    data BYTEA
);


/* 9 */
CREATE TYPE n3250_467036_schema_lab1.n3250_467036_enum AS ENUM (
    'большой',
    'средний',
    'маленький'
);


/* 10 */
CREATE DOMAIN n3250_467036_schema_lab1.n3250_467036_domain AS VARCHAR(3)
CHECK (VALUE ~ '^[0-9]{3}$');


/* 11 */
CREATE SEQUENCE n3250_467036_schema_lab1.n3250_467036_seq
START 1000
INCREMENT -1
MINVALUE 1
MAXVALUE 1000;


/* 12 */
CREATE INDEX n3250_467036_idx1
ON n3250_467036_schema_lab1.n3250_467036_table_i5 (num);


/* 13 */
CREATE INDEX n3250_467036_idx2
ON n3250_467036_schema_lab1.n3250_467036_table_i5 (num, birth_date);


/* 14 */
CREATE INDEX n3250_467036_idx3
ON n3250_467036_schema_lab1.n3250_467036_table_i5 ((num % 10));


/* 15 */
CREATE INDEX n3250_467036_idx4
ON n3250_467036_schema_lab1.n3250_467036_table_i5 (num)
WHERE 100 <= num AND num <= 1000;


/* 16 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_notnull (
    num INTEGER,
    name  VARCHAR(30),
    code CHAR(5) NOT NULL,
    create_date TIMESTAMP,
    birth_date DATE,
    file_data BYTEA
);


/* 17 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_unique (
    num INTEGER,
    name  VARCHAR(30),
    code CHAR(5),
    create_date TIMESTAMP,
    birth_date DATE,
    file_data BYTEA,
    UNIQUE (num, code)
);


/* 18 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_pk (
    num INTEGER PRIMARY KEY,
    name  VARCHAR(30),
    code CHAR(5),
    create_date TIMESTAMP,
    birth_date DATE,
    file_data BYTEA
);


/* 19 */
CREATE TABLE n3250_467036_schema_lab1.n3250_467036_check (
    num INTEGER PRIMARY KEY,
    name  VARCHAR(30) CHECK (name LIKE '%a%'),
    code CHAR(5),
    create_date TIMESTAMP,
    birth_date DATE,
    file_data BYTEA
);


/* 20 */
CREATE VIEW n3250_467036_schema_lab1.n3250_467036_view AS
SELECT name, birth_date FROM n3250_467036_schema_lab1.n3250_467036_table_i5;

\c n3250_467036
\dn
\dt n3250_467036_schema_lab1.*
\dT n3250_467036_schema_lab1.*
\dD n3250_467036_schema_lab1.*
\di n3250_467036_schema_lab1.*
\dv n3250_467036_schema_lab1.*
