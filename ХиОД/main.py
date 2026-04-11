import os
import platform
from src.login import *
import psycopg2
from tabulate import tabulate

# Очистить консоль вывода
def clear_console():
    if platform.system() == "Windows":
        os.system('cls')
    else:
        os.system('clear')

# Вывести данные таблицей
def print_data(cursor, data):
    headers = get_table_headers(cursor)
    print(tabulate(data, headers=headers, tablefmt="grid"))

# Получить заголовки таблицы
def get_table_headers(cursor):
    return [desc[0] for desc in cursor.description]

# Запросить данные
def request(command, option):
    try:
        with psycopg2.connect(
            dbname=dbname,
            user=user,
            password=password,
            host=host,
            port=port,
        ) as conn:
            with conn.cursor() as cursor:
                cursor.execute(command)
                data = cursor.fetchall()
                if option == "all":
                   print_data(cursor, data)
                if option == "r": 
                    if len(data) > 200:
                        start = 0
                        end = 200
                        while True:
                            print_data(cursor, data[start:end])
                            line = input(f"> Строки {start} - {end}  (нажатие ENTER - следующие строки, exit - выход) >>> ")
                            if line == "":
                                start = end
                                end += 200
                                if end > len(data):
                                    end = len(data)
                                    print_data(cursor, data[start:end])
                                    line = input(f"> Строки {start} - {end}  (нажатие ENTER - следующие строки, exit - выход) >>> ")
                                    break
                            if line == "exit":
                                break
                    else:
                        print_data(cursor, data)

    except Exception as e:
        print("\033[91mОшибка при выполнении запроса:")
        print(f"Тип ошибки: {type(e).__name__}")
        print(f"Сообщение: {str(e)}")
        print("\033[0m")
        raise


tip = """Команды:
    exit - выйти
    clear - очистить вывод
    del - отменить запрос и ввести новый
    all - отправить введенный запрос и вывести все данные
    r - отправить введенный запрос и выводить частями
    help - показать эту инструкцию
"""
print(tip)

lines = []

while True:
    line = input("> " if not lines else "... ")

    if line.lower() == "exit":
        break

    if line.lower() == "clear":
        clear_console()
        continue

    if line.lower() == "del":
        print("Ввод запроса отменен. Начните вводить новый запрос:")
        lines = []
        continue

    if line.lower() == "help":
        print(tip)
        continue

    if line.lower() == "all":
        command = "\n".join(lines)
        print("Запрос отправлен\n")
        try:
            request(command, "all")
            lines = []
        except Exception as e:
            lines = []
        continue

    if line.lower() == "r":
        command = "\n".join(lines)
        print("Запрос отправлен\n")
        try:
            request(command, "r")
            lines = []
        except Exception as e:
            lines = []
        continue

    lines.append(line)