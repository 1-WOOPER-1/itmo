DROP TABLE IF EXISTS computers;
DROP TABLE IF EXISTS graphics_cards;
DROP TABLE IF EXISTS processors;

/* 1. СОЗДАЕМ ТАБЛИЦЫ С ПОЛЯМИ */
CREATE TABLE IF NOT EXISTS graphics_cards (
    id SERIAL PRIMARY KEY,
    manufacturer VARCHAR(50),
    vram INT,
    cuda_cores INT
);
CREATE TABLE IF NOT EXISTS processors (
    id SERIAL PRIMARY KEY,
    manufacturer VARCHAR(50),
    cores INT,
    threads INT,
    frequency FLOAT
);
CREATE TABLE IF NOT EXISTS computers (
    id SERIAL PRIMARY KEY,
    graphics_card_id INT REFERENCES graphics_cards(id),
    processor_id INT REFERENCES processors(id),
    description TEXT,
    release_date DATE,
    price DECIMAL(10, 2)
);

/* 2. НАПОЛНЕНИЕ ТАБЛИЦ ДАННЫМИ */
INSERT INTO graphics_cards (id, manufacturer, vram, cuda_cores) VALUES 
(1, 'Nvidia', 4, 1000),
(2, 'Nvidia', 6, 1280),
(3, 'AMD', 8, 2048),
(4, 'Intel', 8, 1024),
(5, 'AMD', 12, 2560);

INSERT INTO processors (id, manufacturer, cores, threads, frequency) VALUES 
(1, 'Intel', 4, 8, 3.2),
(2, 'Intel', 6, 12, 3.6),
(3, 'AMD', 8, 16, 4.0),
(4, 'AMD', 12, 24, 4.5),
(5, 'Apple', 8, 8, 3.1);

INSERT INTO computers (id, graphics_card_id, processor_id, description, release_date, price) VALUES 
(1, 2, 5, 'SSS Rank PC', '2022-01-05', 2500.00),
(2, 5, 4, 'Excellent PC', '2023-09-12', 1800.00),
(3, 3, 3, 'Good PC', '2025-12-01', 1200.00),
(4, 1, 2, 'Not bad PC', '2021-08-23', 800.00),
(5, 3, 1, 'Potato PC', '2024-01-30', 500.00);

/* 3. ОБНОВЛЕНИЕ ЗАПИСЕЙ ТАБЛИЦЫ НА ОСНОВЕ ДАННЫХ ИЗ ДРУГОЙ */
UPDATE computers SET description = 'High-End gaming PC'
WHERE graphics_card_id IN (SELECT id FROM graphics_cards WHERE vram > 10);

/* 4. УДАЛЕНИЕ ЗАПИСЕЙ ИЗ ТАБЛИЦЫ НА ОСНОВЕ ДАННЫХ ИЗ ДРУГОЙ */
DELETE FROM computers
WHERE processor_id IN (SELECT id FROM processors WHERE manufacturer = 'Intel');

/* 5. ВЫВОД ЧАСТИ СТОЛБЦОВ ИЗ ТАБЛИЦЫ */
SELECT manufacturer, vram FROM graphics_cards;

/* 6. ВЫВОД НЕСКОЛЬКИХ ЗАПИСЕЙ С ОГРАНИЧЕНИЯМИ */
SELECT * FROM processors WHERE cores >= 8;

-- /* 7. ДЕКАРТОВО ПРОИЗВЕДЕНИЕ ТАБЛИЦ*/
SELECT gc.manufacturer, pr.manufacturer
FROM graphics_cards AS gc, processors AS pr;

/* 8. ВЫВОД ЗАПИСЕЙ НА ОСНОВАНИИ УСЛОВИЯ ИЗ ДРУГОЙ ТАБЛИЦЫ */
SELECT * FROM graphics_cards
WHERE id IN (SELECT graphics_card_id FROM computers WHERE release_date > '2023-01-01');

/* 9. ПРИМЕНЕНИЕ ФУНКЦИЙ АГРЕГИРОВАНИЯ */
SELECT SUM(price), AVG(price), MIN(price), MAX(price) FROM computers;

/* 10. ВЫВОД ЗАПИСЕЙ С СОРТИРОВКОЙ ПО УБЫВАНИЮ */
SELECT * FROM processors ORDER BY frequency DESC;

/* 11. ВЫВОД ЗАПИСЕЙ ВО ВОЗРАСТАНИЮ С ОГРАНИЧЕНИЕМ НА КОЛИЧЕСтВО СТРОК */
SELECT * FROM graphics_cards ORDER BY vram ASC LIMIT 3;

/* 12. АГРЕГИРОВАНИЕ ВЫВОДИМЫХ ЗАПИСЕЙ */
SELECT manufacturer, COUNT(*) FROM graphics_cards GROUP BY manufacturer;

/* 13. ЗАПРОС, ГДЕ ТАБЛИЧНОЕ ВЫРАЖЕНИЕ - ДРУГОЙ ЗАПРОС */
SELECT sub.manufacturer, sub.cores FROM (SELECT * FROM processors WHERE cores > 4) AS sub WHERE sub.manufacturer = 'AMD';
