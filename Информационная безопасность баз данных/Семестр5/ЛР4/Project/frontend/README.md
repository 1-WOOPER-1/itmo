# Frontend — Container Cargo IS

SPA на React + Vite + Tailwind. Логин, просмотр таблиц по роли, создание / изменение / удаление строк.

## Требования

- Node.js 20+
- Запущенный backend на `http://localhost:8000`

## Запуск

```bash
npm install
```

В `.env`:

```env
VITE_API_BASE_URL=http://localhost:8000/api/v1
```

После изменения `.env` перезапустите dev-сервер.

```bash
npm run dev
```

Откройте http://localhost:5173. Без токена редирект на `/login`.

```bash
npm run build    # tsc + vite build
npm run preview
```

## Скрипты

| Команда | Что делает |
|---|---|
| `npm run dev` | Dev-сервер Vite |
| `npm run build` | Проверка типов и сборка |
| `npm run lint` | ESLint |

## Учётные записи

Те же, что у backend: `admin` / `admin123`, `dispatcher` / `dispatcher123`, `balttrade` / `client123`.

Кнопки создания, правки и удаления завязаны на `can_write` / `can_delete` из `GET /tables`. У клиента `client_id` в заказах не показывается — его подставляет API.

## Структура

```
src/
  pages/          login, main
  components/     таблица, диалог строки, список таблиц
  utils/          API-запросы и подписи колонок
  types/          Table, TableInfo, User
```
