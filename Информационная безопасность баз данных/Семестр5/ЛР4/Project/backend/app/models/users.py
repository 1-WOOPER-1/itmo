from typing import Optional

from sqlalchemy import ForeignKey, Integer, String
from sqlalchemy.orm import Mapped, mapped_column

from app.models.base import Base


class AppUser(Base):
    """Учётная запись приложения (не предметная сущность ЛР1)."""

    __tablename__ = "app_users"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    username: Mapped[str] = mapped_column(String(80), unique=True, nullable=False)
    password_hash: Mapped[str] = mapped_column(String(255), nullable=False)
    role: Mapped[str] = mapped_column(String(30), nullable=False)
    client_id: Mapped[Optional[int]] = mapped_column(
        Integer,
        ForeignKey("clients.id", onupdate="CASCADE", ondelete="SET NULL"),
        nullable=True,
    )
