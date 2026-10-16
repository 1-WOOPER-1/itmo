import os
import shutil
import subprocess
import sys
from tkinter import Button, Label, Tk, messagebox


def get_resource_path():
    if getattr(sys, "frozen", False) and hasattr(sys, "_MEIPASS"):
        return sys._MEIPASS
    return os.path.dirname(os.path.abspath(__file__))


def get_desktop_dir():
    try:
        res = subprocess.check_output(["xdg-user-dir", "DESKTOP"], text=True).strip()
        if os.path.exists(res):
            return res
    except Exception:
        pass

    for fallback in [
        os.path.expanduser("~/Desktop"),
        os.path.expanduser("~/Рабочий стол"),
    ]:
        if os.path.exists(fallback):
            return fallback
    return os.path.expanduser("~/Desktop")


BUNDLE_DIR = get_resource_path()
HOME = os.path.expanduser("~")

INSTALL_DIR = os.path.join(HOME, ".local", "share", "lab2")
APPS_DIR = os.path.join(HOME, ".local", "share", "applications")
DESKTOP_DIR = get_desktop_dir()

MENU_DESKTOP_FILE = os.path.join(APPS_DIR, "lab2.desktop")
DESKTOP_SHORTCUT_FILE = os.path.join(DESKTOP_DIR, "lab2.desktop")
CONF_FILE = os.path.join(HOME, ".config", "lab2", "lab2.txt")


def check_previous_install():
    if os.path.exists(CONF_FILE):
        try:
            with open(CONF_FILE, "r") as f:
                lines = f.read().splitlines()
                if len(lines) >= 2 and int(lines[1]) > 0:
                    return True, int(lines[1])
        except Exception:
            pass
    return False, 0


def install():
    try:
        os.makedirs(INSTALL_DIR, exist_ok=True)
        os.makedirs(APPS_DIR, exist_ok=True)
        os.makedirs(DESKTOP_DIR, exist_ok=True)

        for filename in ["main", "uninstaller"]:
            src = os.path.join(BUNDLE_DIR, filename)
            if not os.path.exists(src):
                raise FileNotFoundError(f"Вшитый файл {filename} не найден!")

            dst = os.path.join(INSTALL_DIR, filename)
            shutil.copy2(src, dst)
            os.chmod(dst, 0o755)

        exec_path = os.path.join(INSTALL_DIR, "main")
        desktop_content = f"""[Desktop Entry]
Version=1.0
Type=Application
Name=Лабораторная 2 (Trial)
Comment=Программа с контролем запусков
Exec={exec_path}
Icon=system-lock-screen
Terminal=false
Categories=Utility;Application;
StartupNotify=true
"""
        with open(MENU_DESKTOP_FILE, "w", encoding="utf-8") as f:
            f.write(desktop_content)
        os.chmod(MENU_DESKTOP_FILE, 0o755)

        with open(DESKTOP_SHORTCUT_FILE, "w", encoding="utf-8") as f:
            f.write(desktop_content)
        os.chmod(DESKTOP_SHORTCUT_FILE, 0o755)

        try:
            subprocess.run(
                ["gio", "set", DESKTOP_SHORTCUT_FILE, "metadata::trusted", "true"],
                stderr=subprocess.DEVNULL,
            )
        except Exception:
            pass

        messagebox.showinfo(
            "Успех",
            "Программа успешно установлена!\n"
            "• Добавлена в меню KDE\n"
            "• Ярлык создан на Рабочем столе",
        )
        root.destroy()
    except Exception as e:
        messagebox.showerror("Ошибка", f"Не удалось установить: {e}")


root = Tk()
root.title("Установка Лабораторной №2")
root.geometry("460x230")
root.resizable(False, False)

was_installed, launches = check_previous_install()

if was_installed:
    lbl_status = Label(
        root,
        text=f"Внимание!\nПрограмма УЖЕ была установлена на этом ПК.\nИспользовано запусков ранее: {launches}.\nЛимиты не будут сброшены!",
        fg="darkorange",
        font=("sans-serif", 10, "bold"),
    )
else:
    lbl_status = Label(
        root,
        text="Первичная установка программы в систему KDE.\nБудут созданы ярлыки в меню и на рабочем столе.",
        fg="green",
    )

lbl_status.pack(pady=15)
btn_install = Button(root, text="Установить", width=22, height=2, command=install)
btn_install.pack(pady=10)

root.mainloop()
