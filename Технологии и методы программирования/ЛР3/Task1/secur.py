#!/usr/bin/env python3
import sys
import os
import base64
import configparser
from pathlib import Path
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel, 
    QLineEdit, QPushButton, QMessageBox, QTextEdit
)
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import serialization

class SecurApp(QWidget):
    def __init__(self, target_file=None):
        super().__init__()
        self.target_file = target_file
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Система защиты secur")
        self.resize(450, 200)

        layout = QVBoxLayout()
        self.label = QLabel("Введите имя раздела (Фамилию студента):")
        layout.addWidget(self.label)

        self.input_surname = QLineEdit()
        self.input_surname.setPlaceholderText("Например: Ivanov")
        layout.addWidget(self.input_surname)

        self.btn_verify = QPushButton("Проверить подпись и открыть sys.tat")
        self.btn_verify.clicked.connect(self.verify_and_open)
        layout.addWidget(self.btn_verify)

        self.setLayout(layout)

    def verify_and_open(self):
        surname = self.input_surname.text().strip()
        if not surname:
            QMessageBox.warning(self, "Ошибка", "Укажите фамилию!")
            return

        config_path = Path.home() / ".config" / surname / "config.ini"
        if not config_path.exists():
            QMessageBox.critical(self, "Отказ в доступе", f"Раздел '{surname}' не найден в реестре/конфиге!")
            sys.exit(1)

        config = configparser.ConfigParser()
        config.read(config_path)

        try:
            signature_b64 = config.get(surname, "Signature")
            signature = base64.b64decode(signature_b64)
        except Exception:
            QMessageBox.critical(self, "Отказ в доступе", "Параметр 'Signature' поврежден или отсутствует!")
            sys.exit(1)

        file_path = self.target_file
        if not file_path or not os.path.exists(file_path):
            exe_dir = Path(os.path.dirname(os.path.abspath(sys.argv[0])))
            file_path = exe_dir / "sys.tat"

        if not Path(file_path).exists():
            QMessageBox.critical(self, "Ошибка", f"Файл не найден:\n{file_path}")
            sys.exit(1)

        with open(file_path, "rb") as f:
            encoded_content = f.read()

        key_path = Path(file_path).parent / "public_key.pem"
        if not key_path.exists():
            exe_dir = Path(os.path.dirname(os.path.abspath(sys.argv[0])))
            key_path = exe_dir / "public_key.pem"

        if not key_path.exists():
            QMessageBox.critical(self, "Ошибка", "Открытый ключ public_key.pem не найден!")
            sys.exit(1)

        with open(key_path, "rb") as kf:
            public_key = serialization.load_pem_public_key(kf.read())

        try:
            public_key.verify(
                signature,
                encoded_content,
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )
        except Exception:
            QMessageBox.critical(
                self, 
                "Нарушение целостности", 
                "Неверная цифровая подпись! Доступ запрещен."
            )
            sys.exit(1)

        self.show_content(encoded_content)

    def show_content(self, raw_data):
        self.viewer = QWidget()
        self.viewer.setWindowTitle("Системная информация [sys.tat]")
        self.viewer.resize(550, 350)
        vbox = QVBoxLayout()

        text_edit = QTextEdit()
        text_edit.setReadOnly(True)
        try:
            decoded = base64.b64decode(raw_data).decode("utf-8")
            text_edit.setText(decoded)
        except Exception:
            text_edit.setText("Ошибка при декодировании содержимого!")

        vbox.addWidget(text_edit)
        self.viewer.setLayout(vbox)
        self.viewer.show()
        self.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    target = sys.argv[1] if len(sys.argv) > 1 else None
    window = SecurApp(target_file=target)
    window.show()
    sys.exit(app.exec())