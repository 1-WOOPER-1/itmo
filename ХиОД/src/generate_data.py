from faker import Faker
import json
import datetime
from random import randint, choice


def json_dump(path, data):
    with open(path, "w") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


fake = Faker("ru_RU")

specialties_names = [
    "Терапевт",
    "Хирург",
    "Кардиолог",
    "Невролог",
    "Офтальмолог",
    "Отоларинголог (ЛОР)",
    "Гинеколог",
    "Уролог",
    "Эндокринолог",
    "Педиатр"
]

diagnoses = [
    # Терапевт
    ["Острая респираторная вирусная инфекция (ОРВИ)", "Гипертоническая болезнь", "Бронхит", "Гастрит"],
    # Хирург
    ["Аппендицит", "Грыжа брюшной полости", "Перелом костей конечностей", "Варикозное расширение вен"],
    # Кардиолог
    ["Ишемическая болезнь сердца", "Аритмия", "Сердечная недостаточность", "Инфаркт миокарда"],
    # Невролог
    ["Мигрень", "Остеохондроз позвоночника", "Вегетососудистая дистония", "Последствия инсульта"],
    # Офтальмолог
    ["Близорукость", "Катаракта", "Конъюнктивит", "Глаукома"],
    # Отоларинголог (ЛОР)
    ["Отит", "Гайморит", "Ангина", "Тугоухость"],
    # Гинеколог
    ["Миома матки", "Эндометриоз", "Нарушение менструального цикла", "Воспалительные заболевания органов малого таза"],
    # Уролог
    ["Мочекаменная болезнь", "Простатит", "Цистит", "Аденома предстательной железы"    ],
    # Эндокринолог
    ["Сахарный диабет", "Гипотиреоз", "Ожирение", "Заболевания щитовидной железы"],
    # Педиатр
    ["Атопический дерматит", "Бронхиальная астма", "Рахит", "Острые инфекции (ветрянка, краснуха, корь)"]
]


# Генерация пациентов
patients = []
for i in range(1, 301):
    gender = choice(["М", "Ж"])
    if gender == "М":
        name = fake.last_name_male() + " " + fake.first_name_male() + " " + fake.middle_name_male()
    else:
        name = fake.last_name_female() + " " + fake.first_name_female() + " " + fake.middle_name_female()
    
    patients.append({
        "id": i,
        "name": name,
        "birth_date": fake.date_of_birth(minimum_age=18, maximum_age=90).isoformat(),
        "gender": gender,
        "phone": fake.numerify("79########"),
        "insurance_number": fake.numerify("###########")
    })


# Генерация докторов
doctors = []
for i in range(1, 51):
    doctors.append({
        "id": i,
        "name": fake.last_name_male() + " " + fake.first_name_male() + " " + fake.middle_name_male(),
        "specialty_id": randint(1, 10)
    })


# Генерация специальностей
specialties = []
for i in range(1, 11):
    specialties.append({
        "id": i,
        "name": specialties_names[i-1]
    })


# Генерация записей
dates = sorted(fake.date_between(datetime.date(2022, 1, 1), datetime.datetime.now()) for i in range(1000))
appointments = []
for i in range(1, 1001):
    doctor_id = randint(1, 50)
    specialty_id = 0
    for doc in doctors:
        if doctor_id == doc["id"]:
            specialty_id = doc["specialty_id"]
    appointments.append({
        "id": i,
        "patient_id": randint(1, 300),
        "doctor_id": doctor_id,
        "date": str(dates[i-1]),
        "diagnosis": choice(diagnoses[specialty_id-1])
    })



json_dump("./data/patients.json", patients)
json_dump("./data/doctors.json", doctors)
json_dump("./data/specialties.json", specialties)
json_dump("./data/appointments.json", appointments)
