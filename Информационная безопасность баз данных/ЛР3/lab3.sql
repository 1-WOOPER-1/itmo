DROP TABLE IF EXISTS ram_standards;
DROP TABLE IF EXISTS sales_2024;
DROP TABLE IF EXISTS sales_2025;
DROP TABLE IF EXISTS sales;
DROP TABLE IF EXISTS rams;
DROP TABLE IF EXISTS audit_log;

-- Таблица товаров
CREATE TABLE rams (
  id SERIAL PRIMARY KEY,
  name TEXT,
  standard VARCHAR(10),
  price NUMERIC,
  total_sales_sum NUMERIC DEFAULT 0
);

-- Таблица продаж
CREATE TABLE sales (
  id SERIAL PRIMARY KEY,
  ram_id INT REFERENCES rams(id),
  amount NUMERIC,
  sale_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


INSERT INTO rams (name, standard, price) VALUES 
('ADATA', 'ddr3', 22000),
('HYPERX', 'ddr4', 45000), 
('CRUCIAL', 'ddr5', 70000);

INSERT INTO sales (ram_id, amount, sale_date) VALUES 
(1, 22000, '2024-10-10 10:00:00'),
(1, 10000, '2024-11-12 12:00:00'),
(2, 1000, '2024-11-12 12:00:00'),
(3, 4000, '2024-11-12 12:00:00');






/* 1. */
CREATE OR REPLACE FUNCTION refresh_total_sales() RETURNS void AS $$
BEGIN
  UPDATE rams AS p
  SET total_sales_sum = (
    SELECT SUM(amount)
    FROM sales AS s WHERE p.id = s.ram_id
  );
END;
$$ LANGUAGE plpgsql;



/* 2. */
CREATE OR REPLACE FUNCTION trigger_update_stock() RETURNS trigger AS $$
BEGIN
  UPDATE rams SET total_sales_sum = total_sales_sum + NEW.amount
  WHERE id = NEW.ram_id;
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_after_insert_sale AFTER INSERT
ON sales FOR EACH ROW EXECUTE PROCEDURE trigger_update_stock();



/* 3. */
CREATE TABLE sales_2024 (CHECK (sale_date >= '2024-01-01' AND sale_date < '2025-01-01')) INHERITS (sales);
CREATE TABLE sales_2025 (CHECK (sale_date >= '2025-01-01' AND sale_date < '2026-01-01')) INHERITS (sales);

CREATE OR REPLACE FUNCTION trigger_route_by_year() RETURNS trigger AS $$
BEGIN
  IF (NEW.sale_date >= '2024-01-01' AND NEW.sale_date < '2025-01-01') THEN
    INSERT INTO sales_2024 VALUES (NEW.*);
  ELSIF (NEW.sale_date >= '2025-01-01' AND NEW.sale_date < '2026-01-01') THEN
    INSERT INTO sales_2025 VALUES (NEW.*);
  ELSE
    RAISE EXCEPTION 'Дата вне поддерживаемого диапазона';
  END IF;
  RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_sales_partitioning
BEFORE INSERT ON sales
FOR EACH ROW EXECUTE PROCEDURE trigger_route_by_year();



/* 4. */
CREATE TABLE ram_standards (
  id SERIAL PRIMARY KEY,
  name VARCHAR(5)
);
INSERT INTO ram_standards (name) VALUES
('ddr3'),
('ddr4'),
('ddr5');

CREATE OR REPLACE FUNCTION trigger_check_standard() RETURNS trigger AS $$
BEGIN
  IF NEW.standard NOT IN (SELECT name FROM ram_standards) THEN
    NEW.standard := 'undefined';
  END IF;
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER  trigger_standard
BEFORE INSERT ON rams
FOR EACH ROW EXECUTE PROCEDURE trigger_check_standard();


/* 5. */
CREATE TABLE audit_log (
    id SERIAL PRIMARY KEY,
    table_name TEXT,
    operation TEXT,
    old_data JSONB,
    new_data JSONB,
    changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE OR REPLACE FUNCTION trigger_audit_log() RETURNS trigger AS $$
BEGIN
  IF TG_OP = 'DELETE' THEN
    INSERT INTO audit_log (table_name, operation, old_data)
    VALUES (TG_TABLE_NAME, TG_OP, to_jsonb(OLD));
    RETURN OLD;
  ELSIF TG_OP = 'UPDATE' THEN
    INSERT INTO audit_log (table_name, operation, old_data, new_data)
    VALUES (TG_TABLE_NAME, TG_OP, to_jsonb(OLD), to_jsonb(NEW));
    RETURN NEW;
  ELSIF TG_OP = 'INSERT' THEN
  INSERT INTO audit_log (table_name, operation, new_data)
  VALUES (TG_TABLE_NAME, TG_OP, to_jsonb(NEW));
  RETURN NEW;
  END IF;
  RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_audit_rams
AFTER INSERT OR UPDATE OR DELETE ON rams
FOR EACH ROW EXECUTE PROCEDURE trigger_audit_log();






-- INSERT INTO sales (ram_id, amount, sale_date) VALUES 
-- (3, 3000, '2025-02-02');



-- INSERT INTO sales (ram_id, amount, sale_date) VALUES 
-- (3, 3000, '2025-02-02');

-- 5
-- UPDATE rams SET price = 1000 WHERE id = 3;
-- INSERT INTO rams (name, standard, price) VALUES
-- ('ATERMITER', 'ddr1', 1000);
-- DELETE FROM sales WHERE ram_id = 1;
-- DELETE FROM rams WHERE id = 1;

-- SELECT refresh_total_sales();
-- DELETE FROM sales_2025 WHERE id = 5;
-- SELECT * FROM sales;
-- INSERT INTO sales (ram_id, amount, sale_date) VALUES 
-- (3, 3000, '2025-02-02');



-- SELECT * FROM rams;
-- SELECT * FROM ram_standards;
-- SELECT * FROM sales;
-- SELECT * FROM sales_2024;
-- SELECT * FROM sales_2025;
-- SELECT * FROM audit_log;
