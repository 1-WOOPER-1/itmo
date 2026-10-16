# Backend — Container Cargo IS

FastAPI-сервис с JWT-авторизацией и доступом к таблицам PostgreSQL по ролям.

## Требования

- Python 3.12+
- PostgreSQL, база `container_cargo_db` (предметные таблицы из предыдущих ЛР)

## Запуск

```bash
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
# заполните .env (POSTGRES_*, SECRET_KEY, CORS_ORIGINS)
uvicorn app.main:app --reload --port 8000
```

При старте создаётся таблица `app_users` и демо-учётные записи.

- API: http://localhost:8000/api/v1
- Swagger: http://localhost:8000/docs
- Health: http://localhost:8000/health

## Переменные `.env`

| Переменная | Назначение |
|---|---|
| `POSTGRES_*` | Хост, порт, пользователь, пароль, имя БД |
| `SECRET_KEY` | Ключ подписи JWT |
| `CORS_ORIGINS` | Источники фронта, через запятую |
| `API_V1_STR` | Префикс API, по умолчанию `/api/v1` |

## Демо-пользователи

| Логин | Пароль | Роль |
|---|---|---|
| `balttrade` | `client123` | client (`client_id = 1`) |
| `dispatcher` | `dispatcher123` | dispatcher |
| `admin` | `admin123` | admin |

## Роли

- **client** — свои заказы, счета, представления; запись только в `orders` (`client_id` ставит сервер)
- **dispatcher** — рейсы, суда, порты, контейнеры; запись в `voyages` и `voyage_orders`
- **admin** — все таблицы; запись и удаление во всех предметных таблицах (не во views)

Представления (`view_*`) только для чтения.

## Основные эндпоинты

Все, кроме `/login` и `/health`, требуют `Authorization: Bearer <token>`.

| Метод | Путь | Описание |
|---|---|---|
| `POST` | `/api/v1/login` | OAuth2 form: `username`, `password` |
| `GET` | `/api/v1/me` | Текущий пользователь |
| `GET` | `/api/v1/tables` | Доступные таблицы и права |
| `GET` | `/api/v1/tables/{name}` | Строки |
| `POST` | `/api/v1/tables/{name}` | Создать строку (JSON колонок, без `id`) |
| `PATCH` | `/api/v1/tables/{name}/{id}` | Изменить строку |
| `DELETE` | `/api/v1/tables/{name}/{id}` | Удалить строку |
| `GET` | `/api/v1/reports` | Агрегированный отчёт по роли |
