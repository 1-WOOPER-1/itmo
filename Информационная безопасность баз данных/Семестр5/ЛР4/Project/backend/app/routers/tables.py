from datetime import date, datetime
from decimal import Decimal, InvalidOperation
from typing import Any, Optional

from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy import String, cast, inspect
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import Session
from sqlalchemy.sql.sqltypes import Date, DateTime, Integer, Numeric

from app.access import (
    TABLE_REGISTRY,
    can_delete,
    can_read,
    can_write,
    is_view,
    readable_tables,
)
from app.db import get_db
from app.deps import get_current_user
from app.models import AppUser, Client, Invoice, Order, OrderContainer

router = APIRouter(tags=["tables"])


def _forbidden(detail: str) -> HTTPException:
    return HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail=detail)


def _not_found(detail: str = "Запись не найдена") -> HTTPException:
    return HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail=detail)


def get_model(table_name: str):
    model = TABLE_REGISTRY.get(table_name)
    if model is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Таблица не найдена")
    return model


def serialize_row(obj: Any) -> dict[str, Any]:
    data: dict[str, Any] = {}
    for column in obj.__table__.columns:
        value = getattr(obj, column.name)
        if isinstance(value, Decimal):
            data[column.name] = str(value)
        elif isinstance(value, datetime):
            data[column.name] = value.isoformat(sep=" ")
        elif isinstance(value, date):
            data[column.name] = value.isoformat()
        else:
            data[column.name] = value
    return data


def coerce_value(column, value: Any) -> Any:
    if value is None:
        return None
    col_type = column.type
    try:
        if isinstance(col_type, Integer):
            return int(value)
        if isinstance(col_type, Numeric):
            return Decimal(str(value))
        if isinstance(col_type, DateTime):
            if isinstance(value, datetime):
                return value
            return datetime.fromisoformat(str(value).replace("Z", ""))
        if isinstance(col_type, Date):
            if isinstance(value, date) and not isinstance(value, datetime):
                return value
            return date.fromisoformat(str(value)[:10])
    except (TypeError, ValueError, InvalidOperation) as exc:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail=f"Некорректное значение поля {column.name}",
        ) from exc
    return value


def payload_to_values(model, payload: dict[str, Any], *, skip_pk: bool = True) -> dict[str, Any]:
    mapper = inspect(model)
    pk_names = {col.name for col in mapper.primary_key}
    columns = {col.name: col for col in model.__table__.columns}
    values: dict[str, Any] = {}
    unknown = [key for key in payload if key not in columns]
    if unknown:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail=f"Неизвестные поля: {', '.join(unknown)}",
        )
    for name, raw in payload.items():
        if skip_pk and name in pk_names:
            continue
        values[name] = coerce_value(columns[name], raw)
    return values


def apply_client_scope(query, model, user: AppUser):
    if user.role != "client":
        return query
    if user.client_id is None:
        raise _forbidden("Учётная запись клиента не привязана к записи clients")
    if model is Client:
        return query.filter(Client.id == user.client_id)
    if hasattr(model, "client_id"):
        return query.filter(model.client_id == user.client_id)
    if model is Invoice:
        return query.join(Order, Invoice.order_id == Order.id).filter(Order.client_id == user.client_id)
    if model is OrderContainer:
        return query.join(Order, OrderContainer.order_id == Order.id).filter(
            Order.client_id == user.client_id
        )
    return query


def apply_sort_filter(query, model, sort: Optional[str], order: str, filter_field: Optional[str], filter_value: Optional[str]):
    columns = {col.name: col for col in model.__table__.columns}
    if filter_field:
        if filter_field not in columns:
            raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Неизвестное поле фильтра")
        if filter_value:
            query = query.filter(cast(columns[filter_field], String).ilike(f"%{filter_value}%"))
    if sort:
        if sort not in columns:
            raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Неизвестное поле сортировки")
        column = columns[sort]
        query = query.order_by(column.desc() if order.lower() == "desc" else column.asc())
    return query


def get_pk_column(model):
    pk_cols = list(inspect(model).primary_key)
    if len(pk_cols) != 1:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Операция по идентификатору доступна только для таблиц с одним PK",
        )
    return pk_cols[0]


@router.get("/tables")
def list_available_tables(user: AppUser = Depends(get_current_user)) -> dict:
    names = readable_tables(user.role)
    return {
        "role": user.role,
        "tables": [
            {
                "name": name,
                "kind": "view" if is_view(name) else "table",
                "can_write": can_write(user.role, name),
                "can_delete": can_delete(user.role, name),
            }
            for name in names
        ],
    }


@router.get("/tables/{table_name}")
def list_rows(
    table_name: str,
    sort: Optional[str] = Query(default=None),
    order: str = Query(default="asc"),
    filter_field: Optional[str] = Query(default=None),
    filter_value: Optional[str] = Query(default=None),
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> dict:
    if not can_read(user.role, table_name):
        raise _forbidden("Нет прав на чтение этой таблицы")
    model = get_model(table_name)
    query = apply_client_scope(db.query(model), model, user)
    query = apply_sort_filter(query, model, sort, order, filter_field, filter_value)
    rows = query.all()
    return {
        "table": table_name,
        "items": [serialize_row(row) for row in rows],
        "count": len(rows),
    }


@router.get("/tables/{table_name}/{row_id}")
def get_row(
    table_name: str,
    row_id: str,
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> dict:
    if not can_read(user.role, table_name):
        raise _forbidden("Нет прав на чтение этой таблицы")
    model = get_model(table_name)
    pk = get_pk_column(model)
    query = apply_client_scope(db.query(model), model, user).filter(pk == coerce_value(pk, row_id))
    row = query.first()
    if row is None:
        raise _not_found()
    return {"table": table_name, "item": serialize_row(row)}


@router.post("/tables/{table_name}", status_code=status.HTTP_201_CREATED)
def create_row(
    table_name: str,
    payload: dict[str, Any],
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> dict:
    if is_view(table_name):
        raise _forbidden("Представления доступны только для чтения")
    if not can_write(user.role, table_name):
        raise _forbidden("Нет прав на добавление записей в эту таблицу")
    model = get_model(table_name)
    data = dict(payload)
    if user.role == "client" and table_name == "orders":
        data["client_id"] = user.client_id
    values = payload_to_values(model, data, skip_pk=True)
    row = model(**values)
    db.add(row)
    try:
        db.commit()
    except IntegrityError as exc:
        db.rollback()
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Нарушение ограничения целостности",
        ) from exc
    db.refresh(row)
    return {"table": table_name, "item": serialize_row(row)}


@router.patch("/tables/{table_name}/{row_id}")
def update_row(
    table_name: str,
    row_id: str,
    payload: dict[str, Any],
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> dict:
    if is_view(table_name):
        raise _forbidden("Представления доступны только для чтения")
    if not can_write(user.role, table_name):
        raise _forbidden("Нет прав на изменение этой таблицы")
    model = get_model(table_name)
    pk = get_pk_column(model)
    query = apply_client_scope(db.query(model), model, user).filter(pk == coerce_value(pk, row_id))
    row = query.first()
    if row is None:
        raise _not_found()
    data = dict(payload)
    if user.role == "client" and table_name == "orders":
        data["client_id"] = user.client_id
    values = payload_to_values(model, data, skip_pk=True)
    for key, value in values.items():
        setattr(row, key, value)
    try:
        db.commit()
    except IntegrityError as exc:
        db.rollback()
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Нарушение ограничения целостности",
        ) from exc
    db.refresh(row)
    return {"table": table_name, "item": serialize_row(row)}


@router.delete("/tables/{table_name}/{row_id}")
def delete_row(
    table_name: str,
    row_id: str,
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> dict:
    if is_view(table_name):
        raise _forbidden("Представления доступны только для чтения")
    if not can_delete(user.role, table_name):
        raise _forbidden("Нет прав на удаление записей из этой таблицы")
    model = get_model(table_name)
    pk = get_pk_column(model)
    query = apply_client_scope(db.query(model), model, user).filter(pk == coerce_value(pk, row_id))
    row = query.first()
    if row is None:
        raise _not_found()
    db.delete(row)
    try:
        db.commit()
    except IntegrityError as exc:
        db.rollback()
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Нельзя удалить запись из-за связанных данных",
        ) from exc
    return {"table": table_name, "deleted_id": row_id}
