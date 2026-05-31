import psycopg2
from login import *

# Подключение к базе данных
with psycopg2.connect(
    dbname=dbname,
    user=user,
    password=password,
    host=host,
    port=port,
) as conn:
    with conn.cursor() as cursor:

        # Создание таблицы с пациентами
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS patients (
                id SERIAL PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                birth_date DATE,
                gender CHAR(1),
                phone VARCHAR(20),
                insurance_number VARCHAR(11)
            )
        """)

        # Создание таблицы со специальностями
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS specialties (
                id SERIAL PRIMARY KEY,
                name VARCHAR(30) NOT NULL
            )
        """)

        # Создание таблицы с докторами
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS doctors (
                id SERIAL PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                specialty_id INT REFERENCES specialties(id) NOT NULL
            )
        """)
        
        # Создание таблицы с записями
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS appointments (
                id SERIAL PRIMARY KEY,
                patient_id INT REFERENCES patients(id) NOT NULL,
                doctor_id INT REFERENCES doctors(id) NOT NULL,
                date DATE NOT NULL,
                diagnosis TEXT
            )
        """)
