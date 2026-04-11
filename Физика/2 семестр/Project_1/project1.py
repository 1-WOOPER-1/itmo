from tkinter import *
import numpy as np


def get_angle(value):
    global alpha_deg
    alpha_deg = float(value)
    alpha_deg = min(max(alpha_deg, 1), 90)
    update_entry_value(entry_angle, round(alpha_deg))
    draw_trajectory()

def get_velocity(value):
    global v0
    v0 = float(value)
    v0 = min(max(v0, 1), 600)
    update_entry_value(entry_velocity, round(v0))
    draw_trajectory()

def get_target_distance(value):
    global l
    l = float(value)
    l = min(max(l, 10), 35_000)
    update_entry_value(entry_target_distance, round(l))
    draw_trajectory()

def get_air_resistance(value):
    global k
    k = float(value)
    k = min(max(k, 0), 100)
    update_entry_value(entry_air_resistance, round(k))
    k /= 100
    draw_trajectory()

def update_entry_value(entry, value):
    entry.delete(0, END)
    entry.insert(0, str(value))

def get_values_from_entries(event):
    get_angle(entry_angle.get())
    get_velocity(entry_velocity.get())
    get_target_distance(entry_target_distance.get())
    get_air_resistance(entry_air_resistance.get())

def change_scale(value):
    global scale
    scale = float(value) / 30
    draw_trajectory()

def calculate_points():
    alpha = np.radians(alpha_deg)
    x, y = 0.0, 0.0
    vx = v0 * np.cos(alpha)
    vy = v0 * np.sin(alpha)
    dt = 0.01
    pts = [(x, y)]
    while y >= 0:
        v = np.hypot(vx, vy)
        ax = -k * vx
        ay = -g - k * vy
        vx += ax * dt
        vy += ay * dt
        x += vx * dt
        y += vy * dt
        pts.append((x, y))
    return pts, pts[-1][0]

def transform_point(x, y):
    global scale
    cx = MARGIN + x * scale
    cy = HEIGHT - (MARGIN + y * scale)
    return cx, cy

def draw_trajectory():
    surface_canvas.delete("all")
    surface_canvas.create_line(MARGIN, HEIGHT-MARGIN, WIDTH-MARGIN, HEIGHT-MARGIN, arrow=LAST)
    surface_canvas.create_line(MARGIN, HEIGHT-MARGIN, MARGIN, MARGIN, arrow=LAST)

    raw, last_x = calculate_points()
    ys = [y for x, y in raw]
    max_y_raw = max(ys)
    max_y_pts = 10**10
    pts = []
    for x, y in raw:
        pts_x, pts_y = transform_point(x, y)
        pts.append([pts_x, pts_y])
        if pts_y < max_y_pts:
            max_y_pts = pts_y

    for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
        surface_canvas.create_line(x1, y1, x2, y2, fill="blue", width=2)
    
    tx = MARGIN + l * (WIDTH - 2*MARGIN) / l
    target_x = transform_point(l, 0)[0]
    surface_canvas.create_line(target_x, HEIGHT-MARGIN, target_x, MARGIN, fill="red", dash=(4,2))
    surface_canvas.create_text(target_x, HEIGHT-MARGIN, text=f"Цель: {round(l)} м", anchor="n", fill="red")
    surface_canvas.create_text(pts[-1][0], HEIGHT-MARGIN+13, text=f"{round(last_x)} м", anchor="n", fill="red")
    surface_canvas.create_oval(pts[-1][0]-3, HEIGHT-MARGIN-3, pts[-1][0]+3, HEIGHT-MARGIN+3, fill="red")
    surface_canvas.create_line(MARGIN, max_y_pts, MARGIN+8, max_y_pts, width=3, fill="red")
    surface_canvas.create_text(MARGIN+35, max_y_pts-10, text=f"{round(max_y_raw)} м", anchor="n")
    surface_canvas.create_text(MARGIN-10, MARGIN, text="y")
    surface_canvas.create_text(WIDTH-MARGIN, HEIGHT-MARGIN, text="x", anchor="n")
    surface_canvas.create_text(MARGIN, HEIGHT-MARGIN, text="0", anchor="n")


l = 50          # расстояние до цели (м)
v0 = 20         # начальная скорость снаряда (м/с)
g = 9.81        # ускорение свободного падения (м/с^2)
k = 0.05        # коэффициент сопротивления воздуха
alpha_deg = 45  # угол в градусах
scale = 5       # масштаб

root = Tk()
root.geometry("1600x900")
root.title("Артиллерийская задача")
root.columnconfigure(index=0, weight=2)
root.columnconfigure(index=1, weight=2)
root.rowconfigure(index=0, weight=1)

surface = Frame(borderwidth=1, relief=SOLID)
surface_hint_label = Label(surface, text="Траектория снаряда")
surface_scale_label = Label(surface, text="Масштаб")
surface_scale_slider = Scale(surface, command=change_scale, from_=1, to=100, resolution=0.1, orient='horizontal', length=500)

WIDTH = 1260
HEIGHT = 800
MARGIN = 30
surface_canvas = Canvas(surface, bg="white", width=WIDTH, height=HEIGHT)
surface_canvas.create_line(MARGIN, HEIGHT - MARGIN, WIDTH - MARGIN, HEIGHT - MARGIN, arrow=LAST)
surface_canvas.create_line(MARGIN, HEIGHT - MARGIN, MARGIN, MARGIN, arrow=LAST)


surface.grid(row=0, column=0, sticky=NSEW, padx=5, pady=5)
surface_hint_label.pack()
surface_canvas.pack(fill=BOTH)
surface_scale_slider.pack(side=RIGHT, anchor=N)
surface_scale_label.pack(side=RIGHT, anchor=N)

sliders = Frame(borderwidth=1, relief=SOLID)
slider_angle = Scale(sliders, command=get_angle, from_=0, to=90, length=620)
slider_velocity = Scale(sliders, command=get_velocity, from_=1, to=600, length=620)
slider_target_distanse = Scale(sliders, command=get_target_distance, from_=10, to=35_000, length=620)
slider_air_resistance = Scale(sliders, command=get_air_resistance, from_=0, to=100, length=620)

entry_angle = Entry(sliders, width=6)
entry_velocity = Entry(sliders, width=6)
entry_target_distance = Entry(sliders, width=6)
entry_air_resistance = Entry(sliders, width=6)
entry_angle.insert(0, str(alpha_deg))
entry_velocity.insert(0, str(v0))
entry_target_distance.insert(0, str(l))
entry_air_resistance.insert(0, str(round(k*100)))

label_angle = Label(sliders, text="a")
label_velocity = Label(sliders, text="v0")
label_target_distance = Label(sliders, text="l")
label_air_resistance = Label(sliders, text="k")

sliders.grid(row=0, column=1, sticky=NSEW, padx=5, pady=5)
slider_angle.grid(row=2, column=0, pady=5)
slider_velocity.grid(row=2, column=1, pady=5)
slider_target_distanse.grid(row=2, column=2, pady=5)
slider_air_resistance.grid(row=2, column=3, pady=5)

entry_angle.grid(row=1, column=0)
entry_velocity.grid(row=1, column=1)
entry_target_distance.grid(row=1, column=2)
entry_air_resistance.grid(row=1, column=3)

label_angle.grid(row=0, column=0)
label_velocity.grid(row=0, column=1)
label_target_distance.grid(row=0, column=2)
label_air_resistance.grid(row=0, column=3)

label_tip = Label(sliders, text="Для применения значений,\nвведённых в поля вручную,\nнажмите ENTER", justify="left")
legend_text = """Обозначения:
a - угол стрельбы, градусы
v0 - начальная скорость снаряда, м^с
l - расстояние до цели, м
k - сопротивление воздуха, k*100
"""
label_legend = Label(sliders, text=legend_text, justify="left")
label_tip.grid(row=3, column=0, columnspan=4)
label_legend.grid(row=4, column=0, columnspan=4, padx=10)

root.bind("<Return>", get_values_from_entries)

draw_trajectory()

root.mainloop()
