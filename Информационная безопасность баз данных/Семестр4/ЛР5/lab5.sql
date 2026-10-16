DROP TABLE IF EXISTS accounts;
DROP TABLE IF EXISTS byte_data;


CREATE EXTENSION IF NOT EXISTS pgcrypto;
\dx


/* 1. */
CREATE TABLE accounts (
    username TEXT,
    passwd TEXT,
    keyphrase TEXT
);

INSERT INTO accounts (username, passwd, keyphrase)
VALUES ('user1', crypt('qwerty', gen_salt('md5')), encode(digest('homie', 'sha1'), 'hex'));

SELECT * FROM accounts;

SELECT (passwd = crypt('213', passwd)) as password,
(keyphrase = encode(digest('ewr', 'sha1'), 'hex')) as keyphrase
FROM accounts;

SELECT (passwd = crypt('qwerty', passwd)) as password,
(keyphrase = encode(digest('homie', 'sha1'), 'hex')) as keyphrase
FROM accounts;


/* 2. */
CREATE TABLE byte_data (
    title TEXT,
    secret_data BYTEA
);

INSERT INTO byte_data (title, secret_data) VALUES
('Secret note', pgp_sym_encrypt('my secret data', 'key'));

SELECT * FROM byte_data;

SELECT title, pgp_sym_decrypt(secret_data, 'key') as decrypted_data
FROM byte_data;

SELECT title, pgp_sym_decrypt(secret_data, 'what') as decrypted_data
FROM byte_data;
