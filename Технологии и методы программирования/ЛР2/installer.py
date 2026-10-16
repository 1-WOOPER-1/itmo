import os
import subprocess
from tkinter import Tk, Label, Button, messagebox

def get_desktop_dir():
    """Получение папки Desktop согласно стандарту XDG"""
    try:
        res = subprocess.check_output(["xdg-user-dir", "DESKTOP"], text=True).strip()
        if os.path.exists(res):
            return res
    except Exception:
        pass
    for fallback in [os.path.expanduser("~/Desktop"), os.path.expanduser("~/Рабочий стол")]:
        if os.path.exists(fallback):
            return fallback
    return os.path.expanduser("~/Desktop")

HOME = os.path.expanduser("~")
INSTALL_DIR = os.path.join(HOME, ".local", "share", "lab2")
APPS_DIR = os.path.join(HOME, ".local", "share", "applications")
DESKTOP_DIR = get_desktop_dir()

MENU_DESKTOP_FILE = os.path.join(APPS_DIR, "lab2.desktop")
DESKTOP_SHORTCUT_FILE = os.path.join(DESKTOP_DIR, "lab2.desktop")

def uninstall():
    """Удаление исполняемых компонентов без удаления конфигурации лицензии"""
    confirm = messagebox.askyesno("Удаление", "Вы действительно хотите удалить программу?")
    if not confirm:
        return

    try:
        # 1. Удаление пункта в меню приложений KDE
        if os.path.exists(MENU_DESKTOP_FILE):
            os.remove(MENU_DESKTOP_FILE)

        # 2. Удаление ярлыка с Рабочего стола
        if os.path.exists(DESKTOP_SHORTCUT_FILE):
            os.remove(DESKTOP_SHORTCUT_FILE)

        # 3. Полное удаление директории установленного бинарного ПО
        if os.path.exists(INSTALL_DIR):
            shutil.rmtree(INSTALL_DIR)

        # 4. Обновление баз данных и сброс кэша рабочего окружения KDE
        subprocess.run(["update-desktop-database", APPS_DIR], stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
        for cmd in ["kbuildsycoca6", "kbuildsycoca5"]:
            try:
                subprocess.run([cmd, "--noincremental"], stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
                break
            except FileNotFoundError:
                continue

        # Лицензионный маркер ~/.config/lab2/lab2.txt намеренно НЕ удаляется!

        messagebox.showinfo("Успешно", "Программа и ярлыки удалены с компьютера!")
        root.destroy()
    except Exception as e:
        messagebox.showerror("Ошибка", f"Ошибка при удалении: {e}")

root = Tk()
root.title("Деинсталлятор")
root.geometry("400x180")
root.resizable(False, False)

lbl = Label(root, text="Мастер удаления программы Лабораторной №2.", pady=15)
lbl.pack()

btn = Button(root, text="Деинсталлировать", fg="red", width=20, height=2, command=uninstall)
btn.pack(pady=10)

root.mainloop()
