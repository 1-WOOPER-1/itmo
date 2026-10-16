from datetime import date, datetime
from decimal import Decimal
from typing import Optional

from sqlalchemy import (
    CheckConstraint,
    Date,
    DateTime,
    ForeignKey,
    Integer,
    Numeric,
    String,
    UniqueConstraint,
    func,
)
from sqlalchemy.orm import Mapped, mapped_column, relationship

from app.models.base import Base


# 1. Сущность «Клиент»
class Client(Base):
    __tablename__ = "clients"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    name: Mapped[str] = mapped_column(String(150), nullable=False)
    email: Mapped[str] = mapped_column(String(100), unique=True, nullable=False)
    phone: Mapped[str] = mapped_column(String(30), nullable=False)
    address: Mapped[str] = mapped_column(String(255), nullable=False)

    # Связи
    orders: Mapped[list["Order"]] = relationship(
        back_populates="client",
        cascade="all, delete-orphan",
        passive_deletes=True
    )


# 2. Сущность «Порт»
class Port(Base):
    __tablename__ = "ports"
    __table_args__ = (
        CheckConstraint("dock_count > 0", name="chk_ports_dock_count"),
        CheckConstraint("max_draft > 0", name="chk_ports_max_draft"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    name: Mapped[str] = mapped_column(String(100), nullable=False)
    country: Mapped[str] = mapped_column(String(100), nullable=False)
    dock_count: Mapped[int] = mapped_column(Integer, nullable=False)
    max_draft: Mapped[Decimal] = mapped_column(Numeric(4, 2), nullable=False)

    # Связи с рейсами (отправление и прибытие)
    departing_voyages: Mapped[list["Voyage"]] = relationship(
        foreign_keys="[Voyage.departure_port_id]",
        back_populates="departure_port"
    )
    arriving_voyages: Mapped[list["Voyage"]] = relationship(
        foreign_keys="[Voyage.destination_port_id]",
        back_populates="destination_port"
    )


# 3. Сущность «Судно»
class Ship(Base):
    __tablename__ = "ships"
    __table_args__ = (
        CheckConstraint("deadweight > 0", name="chk_ships_deadweight"),
        CheckConstraint("year_built >= 1950", name="chk_ships_year_built"),
        CheckConstraint("LENGTH(flag_state) >= 2", name="chk_flag_state_length"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    name: Mapped[str] = mapped_column(String(100), nullable=False)
    imo_number: Mapped[str] = mapped_column(String(20), unique=True, nullable=False)
    deadweight: Mapped[Decimal] = mapped_column(Numeric(10, 2), nullable=False)
    year_built: Mapped[int] = mapped_column(Integer, nullable=False)
    flag_state: Mapped[str] = mapped_column(String(50), default="Россия", nullable=False)

    # Связи
    voyages: Mapped[list["Voyage"]] = relationship(back_populates="ship")


# 4. Сущность «Контейнер»
class Container(Base):
    __tablename__ = "containers"
    __table_args__ = (
        CheckConstraint("empty_weight > 0", name="chk_containers_empty_weight"),
        CheckConstraint("capacity_volume > 0", name="chk_containers_capacity_volume"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    container_number: Mapped[str] = mapped_column(String(20), unique=True, nullable=False)
    type: Mapped[str] = mapped_column(String(50), nullable=False)
    empty_weight: Mapped[Decimal] = mapped_column(Numeric(8, 2), nullable=False)
    capacity_volume: Mapped[Decimal] = mapped_column(Numeric(8, 2), nullable=False)

    # Связи через ассоциативную таблицу
    order_containers: Mapped[list["OrderContainer"]] = relationship(back_populates="container")


# 5. Сущность «Заказ»
class Order(Base):
    __tablename__ = "orders"
    __table_args__ = (
        CheckConstraint("cargo_weight > 0", name="chk_orders_cargo_weight"),
        CheckConstraint("cargo_volume > 0", name="chk_orders_cargo_volume"),
        CheckConstraint("total_price >= 0", name="chk_orders_total_price"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    client_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("clients.id", onupdate="CASCADE", ondelete="RESTRICT"),
        nullable=False
    )
    cargo_name: Mapped[str] = mapped_column(String(150), nullable=False)
    cargo_weight: Mapped[Decimal] = mapped_column(Numeric(10, 2), nullable=False)
    cargo_volume: Mapped[Decimal] = mapped_column(Numeric(10, 2), nullable=False)
    total_price: Mapped[Decimal] = mapped_column(Numeric(12, 2), nullable=False)
    created_at: Mapped[datetime] = mapped_column(DateTime, server_default=func.now(), nullable=False)

    # Связи
    client: Mapped["Client"] = relationship(back_populates="orders")
    invoice: Mapped[Optional["Invoice"]] = relationship(
        back_populates="order",
        uselist=False,
        cascade="all, delete-orphan",
        passive_deletes=True
    )
    order_containers: Mapped[list["OrderContainer"]] = relationship(
        back_populates="order",
        cascade="all, delete-orphan",
        passive_deletes=True
    )
    voyage_orders: Mapped[list["VoyageOrder"]] = relationship(
        back_populates="order",
        cascade="all, delete-orphan",
        passive_deletes=True
    )


# 6. Сущность «Рейс»
class Voyage(Base):
    __tablename__ = "voyages"
    __table_args__ = (
        CheckConstraint("arrival_date > departure_date", name="chk_voyage_dates"),
        CheckConstraint("departure_port_id <> destination_port_id", name="chk_voyage_ports"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    ship_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("ships.id", onupdate="CASCADE", ondelete="RESTRICT"),
        nullable=False
    )
    departure_port_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("ports.id", onupdate="CASCADE", ondelete="RESTRICT"),
        nullable=False
    )
    destination_port_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("ports.id", onupdate="CASCADE", ondelete="RESTRICT"),
        nullable=False
    )
    departure_date: Mapped[datetime] = mapped_column(DateTime, nullable=False)
    arrival_date: Mapped[datetime] = mapped_column(DateTime, nullable=False)
    status: Mapped[str] = mapped_column(String(30), default="Scheduled", nullable=False)

    # Связи
    ship: Mapped["Ship"] = relationship(back_populates="voyages")
    departure_port: Mapped["Port"] = relationship(
        foreign_keys=[departure_port_id],
        back_populates="departing_voyages"
    )
    destination_port: Mapped["Port"] = relationship(
        foreign_keys=[destination_port_id],
        back_populates="arriving_voyages"
    )
    voyage_orders: Mapped[list["VoyageOrder"]] = relationship(
        back_populates="voyage",
        cascade="all, delete-orphan",
        passive_deletes=True
    )


# 7. Сущность «Счет (Инвойс)»
class Invoice(Base):
    __tablename__ = "invoices"
    __table_args__ = (
        CheckConstraint("amount >= 0", name="chk_invoices_amount"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    order_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("orders.id", onupdate="CASCADE", ondelete="CASCADE"),
        unique=True,
        nullable=False
    )
    invoice_number: Mapped[str] = mapped_column(String(50), unique=True, nullable=False)
    amount: Mapped[Decimal] = mapped_column(Numeric(12, 2), nullable=False)
    payment_status: Mapped[str] = mapped_column(String(30), default="Pending", nullable=False)
    issue_date: Mapped[date] = mapped_column(Date, server_default=func.current_date(), nullable=False)

    # Связи
    order: Mapped["Order"] = relationship(back_populates="invoice")


# 8. Связующая сущность «Контейнеры заказа» (M:N)
class OrderContainer(Base):
    __tablename__ = "order_containers"
    __table_args__ = (
        CheckConstraint("gross_weight > 0", name="chk_order_containers_gross_weight"),
        UniqueConstraint("order_id", "container_id", name="uq_order_container"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    order_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("orders.id", onupdate="CASCADE", ondelete="CASCADE"),
        nullable=False
    )
    container_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("containers.id", onupdate="CASCADE", ondelete="RESTRICT"),
        nullable=False
    )
    seal_number: Mapped[str] = mapped_column(String(50), nullable=False)
    gross_weight: Mapped[Decimal] = mapped_column(Numeric(10, 2), nullable=False)

    # Связи
    order: Mapped["Order"] = relationship(back_populates="order_containers")
    container: Mapped["Container"] = relationship(back_populates="order_containers")


# 9. Связующая сущность «Заказы рейса» (M:N)
class VoyageOrder(Base):
    __tablename__ = "voyage_orders"
    __table_args__ = (
        UniqueConstraint("voyage_id", "order_id", name="uq_voyage_order"),
    )

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    voyage_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("voyages.id", onupdate="CASCADE", ondelete="CASCADE"),
        nullable=False
    )
    order_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("orders.id", onupdate="CASCADE", ondelete="CASCADE"),
        nullable=False
    )
    loading_date: Mapped[Optional[datetime]] = mapped_column(DateTime, nullable=True)
    discharge_date: Mapped[Optional[datetime]] = mapped_column(DateTime, nullable=True)

    # Связи
    voyage: Mapped["Voyage"] = relationship(
        "Voyage",
        foreign_keys=[voyage_id],
        back_populates="voyage_orders"
    )
    order: Mapped["Order"] = relationship(back_populates="voyage_orders")