import psycopg2, json
from login import *

def insert_data(cursor, table, path):
    with open(path) as f:
        data = json.load(f)
        for item in data:
            keys = list(item.keys())
            values = list(item.values())
            
            placeholders = ', '.join(['%s'] * len(values))
            
            columns = ', '.join(keys)
            command = f"INSERT INTO {table} ({columns}) VALUES ({placeholders})"

            cursor.execute(command, values)

# Подключение к базе данных
with psycopg2.connect(
    dbname=dbname,
    user=user,
    password=password,
    host=host,
    port=port,
) as conn:
    print("Заполнение данных...")
    with conn.cursor() as cursor:
        # Вставка пациентов
        insert_data(cursor, "patients", "./data/patients.json")
        # Вставка специальностей
        insert_data(cursor, "specialties", "./data/specialties.json")
        # Вставка докторов
        insert_data(cursor, "doctors", "./data/doctors.json")
        # Вставка записей
        insert_data(cursor, "appointments", "./data/appointments.json")
    print("Готово")
