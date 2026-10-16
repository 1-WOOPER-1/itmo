import datetime
import os
import subprocess
from tkinter import Button, Entry, Label, Tk, messagebox
import webbrowser

HOME_PATH = os.path.expanduser("~/.config/lab2")
CONF_FILE = "lab2.txt"
CONF_PATH = f"{HOME_PATH}/{CONF_FILE}"

INSTALL_DIR = os.path.expanduser("~/.local/share/lab2")

# Конфигурация ограничений ознакомительного периода
MAX_LAUNCHES = 3
MAX_LAUNCHES_DELTA = datetime.timedelta(minutes=1)

def is_trial_expired(launches: int, first_launch: datetime.datetime) -> bool:
    """Проверка наступления условий блокировки по счетчику или времени"""
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    return (launches >= MAX_LAUNCHES) or (now - MAX_LAUNCHES_DELTA > first_launch)

def run_uninstaller():
    """Вызов деинсталлятора при блокировке программы"""
    possible_paths = [
        os.path.join(INSTALL_DIR, "uninstaller"),
        os.path.join(INSTALL_DIR, "uninstaller.py"),
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "uninstaller"),
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "uninstaller.py")
    ]

    uninstaller_path = None
    for path in possible_paths:
        if os.path.exists(path):
            uninstaller_path = path
            break

    if uninstaller_path:
        if uninstaller_path.endswith(".py"):
            subprocess.Popen(["python3", uninstaller_path])
        else:
            subprocess.Popen([uninstaller_path])
        root.destroy()
    else:
        messagebox.showerror("Ошибка", "Деинсталлятор не найден в системе!")

def show_error():
    """Переключение графического интерфейса в режим блокировки"""
    global entry, label_error, label_info, btn_uninstall

    error_message = (
        "Вы превысили лимит запусков программы"
        if conf["launches"] >= MAX_LAUNCHES
        else "Вы превысили лимит времени использования программы"
    )

    label_error = Label(text=error_message, fg="red", font=("sans-serif", 9, "bold"))
    label_error.pack(pady=3)

    entry.config(state="disabled")
    button.config(state="disabled")

    label_info.config(
        text="Приобрести лицензию можно у @I_WOOPER_I",
        fg="blue",
        cursor="hand2"
    )
    label_info.bind("<Button-1>", lambda e: webbrowser.open("https://t.me/I_WOOPER_I"))

    btn_uninstall = Button(
        root,
        text="Удалить программу (Деинсталляция)",
        fg="red",
        command=run_uninstaller
    )
    btn_uninstall.pack(pady=5)

def read_conf():
    """Считывание конфигурации и первичная инициализация при первом старте"""
    with open(CONF_PATH, "r") as f:
        conf = f.read().split("\n")

    now = datetime.datetime.now(tz=datetime.timezone.utc)
    if len(conf) >= 3 and len(conf[1]) > 0 and len(conf[2]) > 0:
        new_conf = {
            "name": conf[0],
            "launches": int(conf[1]),
            "first_launch": datetime.datetime.fromisoformat(conf[2])
        }
    else:
        new_conf = {
            "name": "",
            "launches": 0,
            "first_launch": now
        }
    return new_conf

def save_conf():
    """Инкремент счетчика и сохранение ФИО в файл"""
    global entry, conf, label_error, label_info

    if is_trial_expired(conf["launches"], conf["first_launch"]):
        show_error()
        return

    conf["name"] = entry.get() or ""
    conf["launches"] = conf.get("launches", 0) + 1

    with open(CONF_PATH, "w") as f:
        f.write("\n".join(str(v) for v in conf.values()))

    label_info.config(
        text="Данные сохранены!\n" + f"Бесплатных запусков осталось: {MAX_LAUNCHES - conf['launches']}"
    )

    if is_trial_expired(conf["launches"], conf["first_launch"]):
        show_error()

if not os.path.exists(CONF_PATH):
    os.makedirs(HOME_PATH, exist_ok=True)
    with open(CONF_PATH, "w") as f:
        f.write("")
conf = read_conf()

root = Tk()
root.title("Trial Program")
root.geometry("450x290")
root.resizable(False, False)

label_name = Label(
    text=f"Ваше ФИО: {conf['name']}\nИзменить:" if len(conf["name"]) > 0 else "Введите свое ФИО"
)
label_name.pack(pady=10)

entry = Entry()
entry.pack()

button = Button(text="Сохранить", command=save_conf)
button.pack(pady=10)

remaining = MAX_LAUNCHES - conf["launches"]
label_info = Label(
    text=f"Бесплатных запусков осталось: {remaining}" if not is_trial_expired(conf["launches"], conf["first_launch"]) else ""
)
label_info.pack(pady=5)

if is_trial_expired(conf["launches"], conf["first_launch"]):
    show_error()

root.mainloop()
