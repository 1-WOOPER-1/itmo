import psycopg2
from tabulate import tabulate
from login import *

# Вывести данные таблицей
def print_data(cursor, data):
    headers = get_table_headers(cursor)
    print(tabulate(data, headers=headers, tablefmt="grid"))

# Получить заголовки таблицы
def get_table_headers(cursor):
    return [desc[0] for desc in cursor.description]

# Подключение к базе данных
with psycopg2.connect(
    dbname=dbname,
    user=user,
    password=password,
    host=host,
    port=port,
) as conn:
    with conn.cursor() as cursor:

        # Список пациентов поликлиники
        cursor.execute(
            "SELECT * FROM patients"
        )
        data = cursor.fetchall()
        print_data(cursor, data)

        # Список докторов поликлиники
        cursor.execute("""
            SELECT d.id, d.name, s.name
            FROM doctors AS d
            JOIN specialties AS s ON d.specialty_id = s.id
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # Количество пациентов, принятых каждым врачом поликлиники.
        cursor.execute("""
            SELECT a.doctor_id, d.name, COUNT(DISTINCT a.patient_id)
            FROM appointments AS a
            JOIN doctors AS d ON a.doctor_id = d.id
            GROUP BY a.doctor_id, d.name
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # Количество пациентов, принятых каждым специалистом (например, окулистом - 50 человек; из них Иванов принял 23, Петров - 27).
        cursor.execute("""
            SELECT s.name, d.name, COUNT(DISTINCT a.patient_id)
            FROM appointments AS a
            JOIN doctors AS d ON a.doctor_id = d.id
            JOIN specialties AS s ON d.specialty_id = s.id
            GROUP BY s.name, d.name
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # Сколько раз каждый пациент обращался в поликлинику в текущем году
        cursor.execute("""
            SELECT p.id, p.name, COUNT(a.id)
            FROM patients AS p
            LEFT JOIN appointments AS a ON p.id = a.patient_id 
                AND EXTRACT(YEAR FROM a.date) = EXTRACT(YEAR FROM CURRENT_DATE)
            GROUP BY p.id, p.name ORDER BY p.id
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # Сколько раз каждый пациент обращался в поликлинику в текущем месяце
        cursor.execute("""
            SELECT p.id, p.name, COUNT(a.id)
            FROM patients AS p
            LEFT JOIN appointments AS a ON p.id = a.patient_id 
                AND EXTRACT(YEAR FROM a.date) = EXTRACT(YEAR FROM CURRENT_DATE)
                AND EXTRACT(MONTH FROM a.date) = EXTRACT(MONTH FROM CURRENT_DATE)
            GROUP BY p.id, p.name ORDER BY p.id
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # К каким специалистам обращался пациент в прошлом году
        cursor.execute("""
            SELECT p.id, p.name, s.name, COUNT(a.id)
            FROM patients AS p
            LEFT JOIN appointments AS a ON p.id = a.patient_id
                AND EXTRACT(YEAR FROM a.date) = EXTRACT(YEAR FROM CURRENT_DATE) - 1
            LEFT JOIN doctors AS d ON a.doctor_id = d.id
            LEFT JOIN specialties AS s ON d.specialty_id = s.id
            GROUP BY p.id, p.name, s.name
            HAVING COUNT(a.id) > 0
            ORDER BY p.name, COUNT(a.id) DESC
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # К каким специалистам обращался пациент в прошлом месяце
        cursor.execute("""
            SELECT p.id, p.name, s.name, COUNT(a.id)
            FROM patients p
            LEFT JOIN appointments a ON p.id = a.patient_id
                AND (
                    -- Если текущий месяц не январь (просто отнимаем 1 месяц)
                    (EXTRACT(MONTH FROM CURRENT_DATE) > 1 
                    AND EXTRACT(YEAR FROM a.date) = EXTRACT(YEAR FROM CURRENT_DATE)
                    AND EXTRACT(MONTH FROM a.date) = EXTRACT(MONTH FROM CURRENT_DATE) - 1)
                    OR
                    -- Если текущий месяц январь (берем декабрь прошлого года)
                    (EXTRACT(MONTH FROM CURRENT_DATE) = 1
                    AND EXTRACT(YEAR FROM a.date) = EXTRACT(YEAR FROM CURRENT_DATE) - 1
                    AND EXTRACT(MONTH FROM a.date) = 12)
                )
            LEFT JOIN doctors d ON a.doctor_id = d.id
            LEFT JOIN specialties s ON d.specialty_id = s.id
            GROUP BY p.id, p.name, s.name
            HAVING COUNT(a.id) > 0
            ORDER BY p.name, COUNT(a.id) DESC
        """)
        data = cursor.fetchall()
        print_data(cursor, data)

        # Количество врачей у каждой специальности
        cursor.execute("""
            SELECT s.id, s.name, COUNT(DISTINCT d.id)
            FROM specialties AS s JOIN doctors AS d ON s.id = d.specialty_id
            GROUP BY s.id, s.name
        """)
        data = cursor.fetchall()
        print_data(cursor, data)
