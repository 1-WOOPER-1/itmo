from app.models import (
    Client,
    Container,
    Invoice,
    Order,
    OrderContainer,
    Port,
    Ship,
    ViewClientOrder,
    ViewClientTracking,
    ViewContainerSafetyAudit,
    ViewVoyageManifest,
    Voyage,
    VoyageOrder,
)

DOMAIN_TABLES = {
    "clients": Client,
    "ports": Port,
    "ships": Ship,
    "containers": Container,
    "orders": Order,
    "voyages": Voyage,
    "invoices": Invoice,
    "order_containers": OrderContainer,
    "voyage_orders": VoyageOrder,
}

VIEWS = {
    "view_client_orders": ViewClientOrder,
    "view_client_tracking": ViewClientTracking,
    "view_voyage_manifest": ViewVoyageManifest,
    "view_container_safety_audit": ViewContainerSafetyAudit,
}

TABLE_REGISTRY = {**DOMAIN_TABLES, **VIEWS}

ROLE_ACCESS = {
    "client": {
        "read": [
            "view_client_orders",
            "view_client_tracking",
            "orders",
            "invoices",
            "order_containers",
            "clients",
        ],
        "write": ["orders"],
        "delete": [],
    },
    "dispatcher": {
        "read": [
            "view_voyage_manifest",
            "view_container_safety_audit",
            "voyages",
            "ships",
            "ports",
            "containers",
            "voyage_orders",
        ],
        "write": ["voyages", "voyage_orders"],
        "delete": [],
    },
    "admin": {
        "read": list(TABLE_REGISTRY),
        "write": list(DOMAIN_TABLES),
        "delete": list(DOMAIN_TABLES),
    },
}


def readable_tables(role: str) -> list[str]:
    return list(ROLE_ACCESS.get(role, {}).get("read", []))


def can_read(role: str, table_name: str) -> bool:
    return table_name in ROLE_ACCESS.get(role, {}).get("read", [])


def can_write(role: str, table_name: str) -> bool:
    return table_name in ROLE_ACCESS.get(role, {}).get("write", [])


def can_delete(role: str, table_name: str) -> bool:
    return table_name in ROLE_ACCESS.get(role, {}).get("delete", [])


def is_view(table_name: str) -> bool:
    return table_name in VIEWS
