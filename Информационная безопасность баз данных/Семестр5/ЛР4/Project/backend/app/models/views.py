from datetime import datetime
from decimal import Decimal
from typing import Optional

from sqlalchemy import DateTime, Integer, Numeric, String
from sqlalchemy.orm import Mapped, mapped_column

from app.models.base import Base


# 1. Представление: Реестр заказов и оплат клиента
# Создано в ЛР 2, п. 5.1
class ViewClientOrder(Base):
    __tablename__ = "view_client_orders"
    __table_args__ = {"info": {"is_view": True}}

    order_id: Mapped[int] = mapped_column(Integer, primary_key=True)
    client_id: Mapped[int] = mapped_column(Integer)
    cargo_name: Mapped[str] = mapped_column(String(150))
    total_price: Mapped[Decimal] = mapped_column(Numeric(12, 2))
    invoice_number: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    payment_status: Mapped[Optional[str]] = mapped_column(String(30), nullable=True)
    containers_count: Mapped[int] = mapped_column(Integer)
    actual_gross_weight: Mapped[Decimal] = mapped_column(Numeric(10, 2))


# 2. Представление: Маршрутный трекинг груза
# Создано в ЛР 2, п. 5.1
class ViewClientTracking(Base):
    __tablename__ = "view_client_tracking"
    __table_args__ = {"info": {"is_view": True}}

    order_id: Mapped[int] = mapped_column(Integer, primary_key=True)
    client_id: Mapped[int] = mapped_column(Integer)
    cargo_name: Mapped[str] = mapped_column(String(150))
    vessel_name: Mapped[str] = mapped_column(String(100))
    port_of_loading: Mapped[str] = mapped_column(String(100))
    port_of_discharge: Mapped[str] = mapped_column(String(100))
    departure_date: Mapped[datetime] = mapped_column(DateTime)
    arrival_date: Mapped[datetime] = mapped_column(DateTime)
    voyage_status: Mapped[str] = mapped_column(String(30))


# 3. Представление: Судовой грузовой манифест (для диспетчера)
# Создано в ЛР 2, п. 5.2
class ViewVoyageManifest(Base):
    __tablename__ = "view_voyage_manifest"
    __table_args__ = {"info": {"is_view": True}}

    voyage_id: Mapped[int] = mapped_column(Integer, primary_key=True)
    ship_name: Mapped[str] = mapped_column(String(100))
    ship_capacity_tons: Mapped[Decimal] = mapped_column(Numeric(10, 2))
    origin_port: Mapped[str] = mapped_column(String(100))
    destination_port: Mapped[str] = mapped_column(String(100))
    total_orders_assigned: Mapped[int] = mapped_column(Integer)
    total_cargo_tons: Mapped[Decimal] = mapped_column(Numeric(10, 2))


# 4. Представление: Ведомость контроля веса и пломб (аудит безопасности)
# Создано в ЛР 2, п. 5.2
class ViewContainerSafetyAudit(Base):
    __tablename__ = "view_container_safety_audit"
    __table_args__ = {"info": {"is_view": True}}

    # В представлении нет отдельного суррогатного PK, используем пломбу как идентификатор строки
    seal_number: Mapped[str] = mapped_column(String(50), primary_key=True)
    container_number: Mapped[str] = mapped_column(String(20))
    container_type: Mapped[str] = mapped_column(String(50))
    tare_weight: Mapped[Decimal] = mapped_column(Numeric(8, 2))
    gross_weight: Mapped[Decimal] = mapped_column(Numeric(10, 2))
    net_cargo_weight: Mapped[Decimal] = mapped_column(Numeric(10, 2))
    order_id: Mapped[int] = mapped_column(Integer)
    shipper_name: Mapped[str] = mapped_column(String(150))