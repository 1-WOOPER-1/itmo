from PIL import Image
import os

images = [file for file in list(next(os.walk(".")))[2] if ".png" and "chart" in file in file]
print(images)

if images != []:
    for filename in images:
        image = Image.open(filename)

        image = image.convert("RGBA")
        data = image.getdata()

        # Создаем новый список пикселей с прозрачностью вместо белого фона
        new_data = []
        for item in data:
            # Если цвет белый (или близкий к белому), делаем его прозрачным
            if item[:3] == (255, 255, 255):
                new_data.append((255, 255, 255, 0))  # Прозрачный пиксель
            else:
                new_data.append(item)  # Оставляем оригинальный цвет

        # Обновляем изображение и сохраняем
        image.putdata(new_data)
        transparent_image_path = filename
        image.save(transparent_image_path, "PNG")

        # Выводим путь к новому изображению
        transparent_image_path
