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
    # Уделение всех таблиц
    with conn.cursor() as cursor:
        cursor.execute("DROP TABLE IF EXISTS appointments")
        cursor.execute("DROP TABLE IF EXISTS doctors")
        cursor.execute("DROP TABLE IF EXISTS specialties")
        cursor.execute("DROP TABLE IF EXISTS patients")
