from typing import Any, Optional

from pydantic import BaseModel, ConfigDict, Field


class Token(BaseModel):
    access_token: str
    token_type: str = "bearer"


class CurrentUser(BaseModel):
    username: str
    role: str
    client_id: Optional[int] = None
    tables: list[str]


class RowPayload(BaseModel):
    model_config = ConfigDict(extra="allow")


class TableListResponse(BaseModel):
    table: str
    items: list[dict[str, Any]]
    count: int


class ReportResponse(BaseModel):
    role: str
    title: str
    description: str
    items: list[dict[str, Any]] = Field(default_factory=list)
    count: int = 0
