import psycopg2
from tabulate import tabulate

DB_CONFIG = {
    "dbname": "dbsecurity", 
    "user": "postgres",
    "password": "12345",
    "host": "localhost",
    "port": "5432"
}


conn = psycopg2.connect(**DB_CONFIG)
cursor = conn.cursor()

with open("lab6.sql") as f:
    sql_script = f.read()

    cursor.execute(sql_script)
    conn.commit()



# ---------------------------------
#             Задание 2
# ---------------------------------
# ВЫБОРКА
cursor.execute("""SELECT full_name, university
FROM students_info WHERE university = 'ITMO';""")

# ВСТАВКА
cursor.execute("""INSERT INTO users (username, password)
VALUES ('psql_coder', 'psql_passwd') RETURNING id;""")
user_id = cursor.fetchall()[0][0]
cursor.execute("""INSERT INTO students_info (user_id, full_name, university)
VALUES (%s, 'Vladimir Pakhomov', 'ITMO');""", (user_id,))

# УДАЛЕНИЕ
cursor.execute("""DELETE FROM students_info
WHERE full_name = 'Ivan Ivanov';""")


# СЛОЖНЫЕ ЗАПРОСЫ
cursor.execute("""SELECT u.username, s.full_name, f.salary_amount
FROM users u JOIN students_info s ON u.id = s.user_id
JOIN finances f ON u.id = f.user_id;
""")
cursor.execute("""SELECT full_name, university
FROM students_info WHERE user_id IN
(SELECT id FROM finances WHERE salary_amount >=
(SELECT AVG(salary_amount) FROM finances));
""")

conn.commit()


# ---------------------------------
#             Задание 3
# ---------------------------------
username = input("Логин: ")
password = input("Пароль: ")
print("\n")

# НЕБЕЗОПАСНЫЙ СПОСОБ
cursor.execute(f"SELECT * FROM users WHERE username = '{username}' AND password = '{password}';")
data = cursor.fetchall()
headers = [desc[0] for desc in cursor.description]
table = tabulate(data, headers, tablefmt="grid")
print("Небезопасный способ:")
if data:
    print(f"Авторизация прошла успешно")
    print(table)
else:
    print("Неверные логин или пароль")

# БЕЗОПАСНЫЙ СПОСОБ
cursor.execute("SELECT * FROM users WHERE username = %s AND password = %s;", (username, password))
data = cursor.fetchall()
print("\nБезопасный способ:")
if data:
    print("Авторизация прошла успешно")
    print(table)
else:
    print("Неверные логин или пароль")



# ---------------------------------
#             Задание 3
# ---------------------------------
# ОПРЕДЕЛЕНИЕ КОЛИЧЕСТВА СТОЛБЦОВ
# cursor.execute("SELECT")


# headers = [desc[0] for desc in cursor.description]
# print(tabulate(data, headers, tablefmt="grid"))
