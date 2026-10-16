#!/usr/bin/env python3
import base64
import configparser
import os
import platform
import shutil
import socket
import subprocess
import sys
import time
import uuid
from pathlib import Path

import psutil
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding, rsa
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import (
    QApplication,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QProgressBar,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


def get_base_dir():
    if getattr(sys, "frozen", False):
        return Path(sys.executable).parent
    return Path(__file__).resolve().parent


class InstallerApp(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Обновление ядра системы")
        self.resize(550, 320)
        layout = QVBoxLayout()

        layout.addWidget(QLabel("<h3>Мастер установки ядра Linux</h3>"))
        layout.addWidget(QLabel("Введите фамилию студента (для реестра/конфига):"))
        self.input_surname = QLineEdit()
        self.input_surname.setText("")
        layout.addWidget(self.input_surname)

        layout.addWidget(QLabel("Выберите каталог для установки (или создайте новый):"))
        folder_layout = QHBoxLayout()
        self.path_input = QLineEdit()
        self.path_input.setText(str(Path.home() / "Desktop"))
        self.btn_browse = QPushButton("Обзор...")
        self.btn_browse.clicked.connect(self.browse_folder)
        folder_layout.addWidget(self.path_input)
        folder_layout.addWidget(self.btn_browse)
        layout.addLayout(folder_layout)

        self.progress_bar = QProgressBar()
        self.progress_bar.setValue(0)
        layout.addWidget(self.progress_bar)

        self.status_label = QLabel("Ожидание запуска...")
        layout.addWidget(self.status_label)

        self.btn_install = QPushButton("Установить обновление")
        self.btn_install.clicked.connect(self.start_installation)
        layout.addWidget(self.btn_install)

        self.setLayout(layout)

    def browse_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Выберите папку")
        if folder:
            self.path_input.setText(folder)

    def start_installation(self):
        target_dir = Path(self.path_input.text().strip())
        surname = self.input_surname.text().strip()

        if not surname:
            QMessageBox.warning(self, "Ошибка", "Укажите фамилию!")
            return

        try:
            target_dir.mkdir(parents=True, exist_ok=True)
        except Exception as e:
            QMessageBox.critical(self, "Ошибка", f"Не удалось создать каталог: {e}")
            return

        self.target_dir = target_dir
        self.surname = surname
        self.btn_install.setEnabled(False)
        self.btn_browse.setEnabled(False)

        self.step = 0
        self.timer = QTimer()
        self.timer.timeout.connect(self.perform_step)
        self.timer.start(150)

    def perform_step(self):
        self.step += 2
        self.progress_bar.setValue(self.step)

        if self.step == 10:
            self.status_label.setText("Звоним Линусу Торвальдсу...")
        elif self.step == 36:
            self.status_label.setText("Запускаем sudo rm -rf / --no-preserve-root")
        elif self.step == 60:
            self.status_label.setText("Патчим KDE2 под FreeBSD...")
        elif self.step == 86:
            self.status_label.setText("Чиним kernel panic...")
        elif self.step >= 100:
            self.timer.stop()
            self.finalize_installation()

    def get_hardware_info(self):
        # 1. Процессор
        cpu_model = platform.machine()
        try:
            with open("/proc/cpuinfo", "r", encoding="utf-8") as f:
                for line in f:
                    if "model name" in line:
                        cpu_model = line.split(":", 1)[1].strip()
                        break
        except Exception:
            pass

        # 2. Материнская плата
        vendor = "Unknown"
        product = "Unknown"
        try:
            vendor = Path("/sys/class/dmi/id/sys_vendor").read_text().strip()
            product = Path("/sys/class/dmi/id/product_name").read_text().strip()
        except Exception:
            pass

        # 3. Видеокарта
        gpu_info = "Не определена"
        try:
            out = subprocess.check_output("lspci", shell=True).decode()
            for line in out.splitlines():
                if "VGA compatible controller" in line or "3D controller" in line:
                    gpu_info = line.split(":", 2)[-1].strip()
                    break
        except Exception:
            pass

        # 4. Диск и память
        ram = psutil.virtual_memory()
        ram_total_gb = round(ram.total / (1024**3), 2)
        ram_used_gb = round(ram.used / (1024**3), 2)

        disk = psutil.disk_usage("/")
        disk_total_gb = round(disk.total / (1024**3), 1)
        disk_free_gb = round(disk.free / (1024**3), 1)

        # 5. Сеть и идентификаторы
        mac_num = uuid.getnode()
        mac_addr = ":".join(
            f"{(mac_num >> elements) & 0xFF:02x}" for elements in range(40, -1, -8)
        )

        machine_id = "N/A"
        if Path("/etc/machine-id").exists():
            machine_id = Path("/etc/machine-id").read_text().strip()

        # 6. Дистрибутив
        distro_name = "Linux"
        try:
            with open("/etc/os-release") as f:
                for line in f:
                    if line.startswith("PRETTY_NAME="):
                        distro_name = line.split("=", 1)[1].strip().strip('"')
        except Exception:
            pass

        uptime_hours = round((time.time() - psutil.boot_time()) / 3600, 1)

        ip_addresses = []
        for interface, addrs in psutil.net_if_addrs().items():
            for addr in addrs:
                if addr.family == socket.AF_INET and not addr.address.startswith(
                    "127."
                ):
                    ip_addresses.append(f"{interface}: {addr.address}")

        return f"""==================================================
СИСТЕМНЫЙ ОТЧЁТ
==================================================

[ОПЕРАЦИОННАЯ СИСТЕМА]
Дистрибутив:      {distro_name}
Версия ядра:      {platform.release()} ({platform.architecture()[0]})
Графическая среда: {os.environ.get("XDG_CURRENT_DESKTOP", "Unknown")} ({os.environ.get("XDG_SESSION_TYPE", "Unknown")})
Время работы:     {uptime_hours} ч. с момента включения

[ПОЛЬЗОВАТЕЛЬ И ХОСТ]
Имя пользователя: {os.getlogin()}
Имя ПК (Hostname):{platform.node()}
Machine ID:       {machine_id}

[АППАРАТНАЯ ЧАСТЬ]
Устройство/Плата: {vendor} {product}
Процессор:        {cpu_model}
Логических ядер:  {psutil.cpu_count(logical=True)} (физических: {psutil.cpu_count(logical=False)})
Оперативная память: {ram_used_gb} GB занято / {ram_total_gb} GB всего
Видеоадаптер:     {gpu_info}
Накопитель (/):   {disk_free_gb} GB свободно / {disk_total_gb} GB всего

[СЕТЕВЫЕ ДАННЫЕ]
MAC-адрес:        {mac_addr}
Сетевые интерфейсы и IP-адреса:

    {"\n    ".join(ip_addresses)}
=================================================="""

    def finalize_installation(self):
        hw_info = self.get_hardware_info()
        encoded_data = base64.b64encode(hw_info.encode("utf-8"))
        sys_tat_path = self.target_dir / "sys.tat"
        with open(sys_tat_path, "wb") as f:
            f.write(encoded_data)

        private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
        public_key = private_key.public_key()

        signature = private_key.sign(
            encoded_data,
            padding.PSS(
                mgf=padding.MGF1(hashes.SHA256()), salt_length=padding.PSS.MAX_LENGTH
            ),
            hashes.SHA256(),
        )
        signature_b64 = base64.b64encode(signature).decode("utf-8")

        config_dir = Path.home() / ".config" / self.surname
        config_dir.mkdir(parents=True, exist_ok=True)
        config_file = config_dir / "config.ini"

        config = configparser.ConfigParser()
        config[self.surname] = {"Signature": signature_b64}
        with open(config_file, "w") as cf:
            config.write(cf)

        base_dir = get_base_dir()

        secur_bin = base_dir / "secur"
        secur_py = base_dir / "secur.py"

        if secur_bin.exists():
            secur_dest = self.target_dir / "secur"
            if secur_bin.resolve() != secur_dest.resolve():
                shutil.copy(secur_bin, secur_dest)
        elif secur_py.exists():
            secur_dest = self.target_dir / "secur.py"
            if secur_py.resolve() != secur_dest.resolve():
                shutil.copy(secur_py, secur_dest)
        else:
            secur_dest = self.target_dir / "secur"
            secur_dest.touch()

        os.chmod(secur_dest, 0o755)

        pub_pem = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo,
        )
        with open(self.target_dir / "public_key.pem", "wb") as pf:
            pf.write(pub_pem)

        self.register_file_association(secur_dest)

        self.status_label.setText("Установка завершена!")
        QMessageBox.information(
            self,
            "Успех",
            f"Обновление успешно установлено!\nФайлы сохранены в:\n{self.target_dir}",
        )

        if secur_dest.suffix == ".py":
            subprocess.Popen([sys.executable, str(secur_dest), str(sys_tat_path)])
        else:
            subprocess.Popen([str(secur_dest), str(sys_tat_path)])

        self.close()

    def register_file_association(self, secur_path):
        home = Path.home()

        mime_dir = home / ".local" / "share" / "mime" / "packages"
        mime_dir.mkdir(parents=True, exist_ok=True)
        xml_content = """<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
    <mime-type type="application/x-tat">
        <comment>TAT Protected Document</comment>
        <glob pattern="*.tat"/>
    </mime-type>
</mime-info>
"""
        with open(mime_dir / "x-tat.xml", "w") as f:
            f.write(xml_content)

        subprocess.run(
            ["update-mime-database", str(home / ".local" / "share" / "mime")],
            check=False,
        )

        apps_dir = home / ".local" / "share" / "applications"
        apps_dir.mkdir(parents=True, exist_ok=True)

        exec_cmd = (
            f"{sys.executable} {secur_path.absolute()} %f"
            if secur_path.suffix == ".py"
            else f"{secur_path.absolute()} %f"
        )

        desktop_content = f"""[Desktop Entry]
Type=Application
Name=Secur TAT Handler
Exec={exec_cmd}
MimeType=application/x-tat;
Terminal=false
NoDisplay=true
"""
        desktop_file = apps_dir / "secur-tat.desktop"
        with open(desktop_file, "w") as f:
            f.write(desktop_content)

        subprocess.run(["update-desktop-database", str(apps_dir)], check=False)
        subprocess.run(
            ["xdg-mime", "default", "secur-tat.desktop", "application/x-tat"],
            check=False,
        )
        shutil.copy(desktop_file, home / "Desktop")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    installer = InstallerApp()
    installer.show()
    sys.exit(app.exec())
