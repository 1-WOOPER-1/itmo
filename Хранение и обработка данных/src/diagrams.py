import psycopg2
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
from src.login import *

# Подключение к базе данных
with psycopg2.connect(
    dbname=dbname,
    user=user,
    password=password,
    host=host,
    port=port,
) as conn:
    with conn.cursor() as cursor:

        # 1. Распределение пациентов по полу (круговая диаграмма)
        cursor.execute("""
            SELECT gender, COUNT(*) as count 
            FROM patients 
            GROUP BY gender
        """)
        gender_data = cursor.fetchall()
        genders = [row[0] for row in gender_data]
        counts = [row[1] for row in gender_data]

        plt.figure(figsize=(6, 6))
        plt.pie(counts, labels=genders, autopct='%1.1f%%', startangle=90)
        plt.title('Распределение пациентов по полу')
        plt.show()

        # 2. Количество записей по врачам (столбчатая диаграмма)
        cursor.execute("""
            SELECT d.name, COUNT(a.id) as appointment_count
            FROM doctors d
            LEFT JOIN appointments a ON d.id = a.doctor_id
            GROUP BY d.name
            ORDER BY appointment_count DESC
        """)
        doctor_data = cursor.fetchall()
        doctors = [row[0] for row in doctor_data]
        appointments = [row[1] for row in doctor_data]

        plt.figure(figsize=(10, 5))
        sns.barplot(x=appointments, y=doctors, palette="viridis")
        plt.title('Количество записей по врачам')
        plt.xlabel('Количество записей')
        plt.ylabel('Врач')
        plt.tight_layout()
        plt.show()

        # 3. Распределение специальностей врачей (горизонтальная столбчатая диаграмма)
        cursor.execute("""
            SELECT s.name, COUNT(d.id) as doctor_count
            FROM specialties s
            LEFT JOIN doctors d ON s.id = d.specialty_id
            GROUP BY s.name
            ORDER BY doctor_count DESC
        """)
        specialty_data = cursor.fetchall()
        specialties = [row[0] for row in specialty_data]
        doctor_counts = [row[1] for row in specialty_data]

        plt.figure(figsize=(8, 4))
        plt.barh(specialties, doctor_counts, color='skyblue')
        plt.title('Распределение врачей по специальностям')
        plt.xlabel('Количество врачей')
        plt.ylabel('Специальность')
        plt.tight_layout()
        plt.show()

        # 4. Динамика записей по месяцам (линейный график)
        cursor.execute("""
            SELECT TO_CHAR(date, 'YYYY-MM') as month, COUNT(*) as count
            FROM appointments
            GROUP BY month
            ORDER BY month
        """)
        monthly_data = cursor.fetchall()
        months = [row[0] for row in monthly_data]
        counts = [row[1] for row in monthly_data]

        plt.figure(figsize=(10, 5))
        plt.plot(months, counts, marker='o')
        plt.title('Динамика количества записей по месяцам')
        plt.xlabel('Месяц')
        plt.ylabel('Количество записей')
        plt.xticks(rotation=45)
        plt.grid(True)
        plt.tight_layout()
        plt.show()

        # 5. Возрастное распределение пациентов (гистограмма)
        cursor.execute("""
            SELECT EXTRACT(YEAR FROM age(birth_date)) as age
            FROM patients
        """)
        age_data = cursor.fetchall()
        ages = [row[0] for row in age_data]

        plt.figure(figsize=(8, 5))
        sns.histplot(ages, bins=20, kde=True, color='green')
        plt.title('Возрастное распределение пациентов')
        plt.xlabel('Возраст')
        plt.ylabel('Количество пациентов')
        plt.tight_layout()
        plt.show()
