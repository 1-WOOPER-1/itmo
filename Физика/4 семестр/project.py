import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# --- 1. Определение базовых матриц ---

def translation_matrix(d, n):
    """Матрица перемещения (трансляции)"""
    return np.array([
        [1.0, 0.0],
        [d / n, 1.0]
    ])

def refraction_matrix(r, n_in, n_out):
    """Матрица преломления на сферической поверхности"""
    if r == 0:
        r = 1e-5  # Предотвращение деления на ноль
    k = (n_out - n_in) / r
    return np.array([
        [1.0, -k],
        [0.0, 1.0]
    ])

# --- 2. Функция расчета траектории луча ---

def trace_ray_visual(x0, alpha0, n1, n2, n3, r1, r2, d_to_lens, d_lens, d_after):
    R1 = refraction_matrix(r1, n1, n2)
    R2 = refraction_matrix(r2, n2, n3)
    
    # Пересечение с первой изогнутой поверхностью
    x_entrance = x0 + alpha0 * d_to_lens
    x_entrance = np.clip(x_entrance, -abs(r1) * 0.99, abs(r1) * 0.99)
    
    dz1 = r1 - np.sign(r1) * np.sqrt(r1**2 - x_entrance**2)
    z1 = d_to_lens + dz1
    x1 = x0 + alpha0 * z1
    
    # Состояние после преломления на 1-й поверхности
    v1 = R1 @ np.array([[n1 * alpha0], [x_entrance]])
    alpha1_prime = v1[0, 0] / n2
    
    # Оценка высоты луча на выходе
    x2_approx = x1 + alpha1_prime * d_lens
    x2_approx = np.clip(x2_approx, -abs(r2) * 0.99, abs(r2) * 0.99)
    
    # Пересечение со второй изогнутой поверхностью
    dz2 = r2 - np.sign(r2) * np.sqrt(r2**2 - x2_approx**2)
    z2 = d_to_lens + d_lens + dz2
    
    # Фактическая высота на второй поверхности
    x2 = x1 + alpha1_prime * (z2 - z1)
    
    # Состояние после преломления на 2-й поверхности
    v2 = R2 @ np.array([[n2 * alpha1_prime], [x2_approx]])
    alpha2 = v2[0, 0] / n3
    
    # Конечная точка
    z_end = d_to_lens + d_lens + d_after
    x_end = x2 + alpha2 * (z_end - z2)
    
    return [0.0, z1, z2, z_end], [x0, x1, x2, x_end]

def get_lens_surface_points(z_vertex, r, h_drawn):
    x = np.linspace(-h_drawn, h_drawn, 100)
    zc = z_vertex + r
    sign = np.sign(r)
    z = zc - sign * np.sqrt(r**2 - x**2)
    return z, x

# --- 3. Настройка графического интерфейса ---

# Задаем более широкое окно для размещения панели расчетов справа
fig = plt.figure(figsize=(14, 8))

# Главный график (слева)
ax = fig.add_axes([0.07, 0.38, 0.70, 0.52])

# Панель расчетов (справа вверху)
ax_calc = fig.add_axes([0.80, 0.38, 0.18, 0.52])

# Оси для слайдеров [лево, низ, ширина, высота]
ax_r1 = plt.axes([0.07, 0.25, 0.23, 0.03])
ax_r2 = plt.axes([0.07, 0.18, 0.23, 0.03])
ax_dlens = plt.axes([0.43, 0.25, 0.23, 0.03])
ax_n2 = plt.axes([0.43, 0.18, 0.23, 0.03])
ax_alpha = plt.axes([0.07, 0.10, 0.59, 0.03])

# Создание слайдеров
s_r1 = Slider(ax_r1, 'Радиус R1', -100.0, 100.0, valinit=25.0, valstep=0.5)
s_r2 = Slider(ax_r2, 'Радиус R2', -100.0, 100.0, valinit=-25.0, valstep=0.5)
s_dlens = Slider(ax_dlens, 'Толщина d', 3.0, 20.0, valinit=6.0, valstep=0.5)
s_n2 = Slider(ax_n2, 'Показатель n', 1.0, 2.0, valinit=1.5, valstep=0.01)
s_alpha = Slider(ax_alpha, 'Угол alpha', -0.15, 0.15, valinit=0.0, valstep=0.005)

# --- 4. Функция обновления системы и вывода расчетов ---

def draw_system():
    # Очистка основного графика
    ax.clear()
    
    # Считываем текущие параметры
    r1 = s_r1.val
    r2 = s_r2.val
    d_lens = s_dlens.val
    n2 = s_n2.val
    alpha0 = s_alpha.val
    
    # Защита от деления на ноль для корректности вычислений
    min_r = 12.0
    if abs(r1) < min_r:
        r1 = min_r * np.sign(r1) if r1 != 0 else min_r
    if abs(r2) < min_r:
        r2 = min_r * np.sign(r2) if r2 != 0 else -min_r
    
    n1 = 1.0
    n3 = 1.0
    d_to_lens = 15.0
    d_after = 60.0
    h_ap = 7.0  # Базовая полуапертура линзы
    
    # Вычисление высоты отрисовки линзы
    h_drawn1 = min(h_ap, abs(r1) * 0.95) if abs(r1) < h_ap else h_ap
    h_drawn2 = min(h_ap, abs(r2) * 0.95) if abs(r2) < h_ap else h_ap
    h_drawn = min(h_drawn1, h_drawn2)
    
    # 4.1. Вычисление оптических сил и матриц
    k1 = (n2 - n1) / r1
    k2 = (n3 - n2) / r2
    
    R1 = refraction_matrix(r1, n1, n2)
    T21 = translation_matrix(d_lens, n2)
    R2 = refraction_matrix(r2, n2, n3)
    S = R2 @ T21 @ R1  # Системная матрица
    
    B = S[0, 1]
    if abs(B) > 1e-5:
        f_prime = -n3 / B
        f_prime_str = f"{f_prime:.2f} мм"
    else:
        f_prime_str = "∞ (афокальная)"
        
    # 4.2. Формирование текста расчетов для правой панели
    calc_text = (
        " ВХОДНЫЕ ПАРАМЕТРЫ:\n"
        f"  R1 = {r1:+.1f} мм,  R2 = {r2:+.1f} мм\n"
        f"  d = {d_lens:.1f} мм,  n = {n2:.2f},  alpha = {np.degrees(alpha0):+.1f}°\n\n"
        " ОПТИЧЕСКАЯ СИЛА ПОВЕРХНОСТЕЙ:\n"
        f"  k1 = (n - 1)/R1 = {k1:+.4f} мм⁻¹\n"
        f"  k2 = (1 - n)/R2 = {k2:+.4f} мм⁻¹\n\n"
        " МАТРИЦЫ СИСТЕМЫ:\n"
        "  R1 (Преломление на 1-й границе):\n"
        f"  [  1.000   {R1[0,1]:.4f} ]\n"
        f"  [  0.000    1.000 ]\n\n"
        "  T21 (Перенос внутри линзы):\n"
        f"  [  1.000    0.000 ]\n"
        f"  [  {T21[1,0]:.4f}    1.000 ]\n\n"
        "  R2 (Преломление на 2-й границе):\n"
        f"  [  1.000   {R2[0,1]:.4f} ]\n"
        f"  [  0.000    1.000 ]\n\n"
        " СИСТЕМНАЯ МАТРИЦА S = R2 * T21 * R1:\n"
        f"  [  {S[0,0]:.4f}   {S[0,1]:.4f} ]\n"
        f"  [  {S[1,0]:.4f}   {S[1,1]:.4f} ]\n\n"
        " ФОКУСНОЕ РАССТОЯНИЕ:\n"
        f"  f' = -1.0 / S[0,1] = {f_prime_str}"
    )
    
    # Обновление информационной панели справа
    ax_calc.clear()
    ax_calc.axis('on')
    ax_calc.set_xticks([])
    ax_calc.set_yticks([])
    ax_calc.patch.set_facecolor('#fdfdfd')      # Светлый фон панели
    ax_calc.patch.set_edgecolor('#d3d3d3')      # Серая рамка
    ax_calc.patch.set_linewidth(1.5)
    
    # Пишем текст моноширинным шрифтом для идеального выравнивания столбцов матриц
    ax_calc.text(0.04, 0.96, calc_text, 
                 family='monospace', fontsize=9.2, 
                 verticalalignment='top', horizontalalignment='left')
    
    # 4.3. Отрисовка линзы
    z1, x1 = get_lens_surface_points(d_to_lens, r1, h_drawn)
    z2, x2 = get_lens_surface_points(d_to_lens + d_lens, r2, h_drawn)
    
    z_poly = np.concatenate([z1, z2[::-1]])
    x_poly = np.concatenate([x1, x2[::-1]])
    ax.fill(z_poly, x_poly, color='deepskyblue', alpha=0.15, edgecolor='deepskyblue', linewidth=2)
    
    # 4.4. Трассировка и отрисовка лучей
    target_heights = np.linspace(-0.8 * h_drawn, 0.8 * h_drawn, 5)
    for x_target in target_heights:
        x0 = x_target - alpha0 * d_to_lens
        z_pts, x_pts = trace_ray_visual(x0, alpha0, n1, n2, n3, r1, r2, d_to_lens, d_lens, d_after)
        ax.plot(z_pts, x_pts, color='red', alpha=0.8, linewidth=1.5)
        
    # Оптическая ось
    ax.axhline(0, color='gray', linestyle='--', linewidth=1)
    
    # Настройки отображения левого графика
    ax.set_xlim(0, d_to_lens + d_lens + d_after)
    ax.set_ylim(-10, 10)
    ax.set_aspect('equal', adjustable='box')
    
    alpha_deg = np.degrees(alpha0)
    ax.set_title(f"Интерактивная модель толстой линзы (Ход параксиальных лучей)\nУгол наклона: {alpha_deg:.1f}°", fontsize=11, pad=10)
    ax.set_xlabel("Координата Z (мм)")
    ax.set_ylabel("Высота X (мм)")
    ax.grid(True, alpha=0.3)
    
    fig.canvas.draw_idle()

# Назначаем функцию обновления на изменение любого слайдера
def on_change(val):
    draw_system()

s_r1.on_changed(on_change)
s_r2.on_changed(on_change)
s_dlens.on_changed(on_change)
s_n2.on_changed(on_change)
s_alpha.on_changed(on_change)

# Первичный запуск отрисовки при старте
draw_system()
plt.show()