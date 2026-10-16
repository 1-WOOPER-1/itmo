from datetime import date, datetime
from decimal import Decimal
from typing import Any, Optional

from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy import text
from sqlalchemy.orm import Session

from app.db import get_db
from app.deps import get_current_user
from app.models import AppUser
from app.schemas import ReportResponse

router = APIRouter(tags=["reports"])

REPORTS = {
    "client": {
        "title": "Сводка заказов клиента по статусу оплаты",
        "description": "Агрегация orders, invoices и order_containers с GROUP BY payment_status.",
        "sql": """
            SELECT
                COALESCE(i.payment_status, 'NoInvoice') AS payment_status,
                COUNT(o.id) AS orders_count,
                COALESCE(SUM(o.total_price), 0) AS total_price,
                COALESCE(SUM(c.containers_count), 0) AS containers_count,
                COALESCE(SUM(c.actual_gross_weight), 0) AS total_gross_weight
            FROM orders o
            LEFT JOIN invoices i ON i.order_id = o.id
            LEFT JOIN (
                SELECT
                    order_id,
                    COUNT(*) AS containers_count,
                    SUM(gross_weight) AS actual_gross_weight
                FROM order_containers
                GROUP BY order_id
            ) c ON c.order_id = o.id
            WHERE o.client_id = :client_id
            GROUP BY COALESCE(i.payment_status, 'NoInvoice')
        """,
    },
    "dispatcher": {
        "title": "Загрузка рейсов относительно дедвейта судна",
        "description": "Агрегация voyages, ships, ports, voyage_orders и orders с GROUP BY рейса.",
        "sql": """
            SELECT
                v.id AS voyage_id,
                s.name AS ship_name,
                s.deadweight AS ship_capacity_tons,
                p1.name AS origin_port,
                p2.name AS destination_port,
                v.status AS voyage_status,
                COUNT(vo.order_id) AS orders_count,
                COALESCE(SUM(o.cargo_weight), 0) / 1000.0 AS total_cargo_tons,
                ROUND(
                    (
                        COALESCE(SUM(o.cargo_weight), 0) / 1000.0
                        / NULLIF(s.deadweight, 0)
                    ) * 100,
                    2
                ) AS load_percent
            FROM voyages v
            JOIN ships s ON v.ship_id = s.id
            JOIN ports p1 ON v.departure_port_id = p1.id
            JOIN ports p2 ON v.destination_port_id = p2.id
            LEFT JOIN voyage_orders vo ON v.id = vo.voyage_id
            LEFT JOIN orders o ON vo.order_id = o.id
            GROUP BY
                v.id, s.name, s.deadweight, p1.name, p2.name, v.status
        """,
    },
    "admin": {
        "title": "Выручка и заказы по клиентам",
        "description": "Агрегация clients, orders и invoices с GROUP BY клиента.",
        "sql": """
            SELECT
                cl.id AS client_id,
                cl.name AS client_name,
                COUNT(o.id) AS orders_count,
                COALESCE(SUM(o.total_price), 0) AS total_revenue,
                COUNT(i.id) FILTER (WHERE i.payment_status = 'Paid') AS paid_invoices,
                COUNT(i.id) FILTER (WHERE i.payment_status = 'Pending') AS pending_invoices,
                COUNT(i.id) FILTER (WHERE i.payment_status = 'Overdue') AS overdue_invoices
            FROM clients cl
            LEFT JOIN orders o ON o.client_id = cl.id
            LEFT JOIN invoices i ON i.order_id = o.id
            GROUP BY cl.id, cl.name
        """,
    },
}


def dump_value(value: Any) -> Any:
    if isinstance(value, Decimal):
        return str(value)
    if isinstance(value, datetime):
        return value.isoformat(sep=" ")
    if isinstance(value, date):
        return value.isoformat()
    return value


def apply_report_params(
    rows: list[dict[str, Any]],
    sort: Optional[str],
    order: str,
    filter_field: Optional[str],
    filter_value: Optional[str],
) -> list[dict[str, Any]]:
    if not rows:
        return rows
    allowed = set(rows[0].keys())
    if filter_field:
        if filter_field not in allowed:
            raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Неизвестное поле фильтра")
        if filter_value:
            needle = filter_value.lower()
            rows = [row for row in rows if needle in str(row.get(filter_field) or "").lower()]
    if sort:
        if sort not in allowed:
            raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Неизвестное поле сортировки")
        reverse = order.lower() == "desc"
        rows = sorted(rows, key=lambda row: (row.get(sort) is None, row.get(sort)), reverse=reverse)
    return rows


@router.get("/reports", response_model=ReportResponse)
def get_role_report(
    sort: Optional[str] = Query(default=None),
    order: str = Query(default="asc"),
    filter_field: Optional[str] = Query(default=None),
    filter_value: Optional[str] = Query(default=None),
    user: AppUser = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> ReportResponse:
    spec = REPORTS.get(user.role)
    if spec is None:
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Для этой роли отчёт не задан")

    params = {}
    if user.role == "client":
        if user.client_id is None:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Учётная запись клиента не привязана к записи clients",
            )
        params["client_id"] = user.client_id

    result = db.execute(text(spec["sql"]), params)
    rows = [{key: dump_value(value) for key, value in dict(item).items()} for item in result.mappings().all()]
    rows = apply_report_params(rows, sort, order, filter_field, filter_value)
    return ReportResponse(
        role=user.role,
        title=spec["title"],
        description=spec["description"],
        items=rows,
        count=len(rows),
    )
