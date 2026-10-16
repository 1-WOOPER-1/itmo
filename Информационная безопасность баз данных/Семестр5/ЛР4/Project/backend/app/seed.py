from app.db import SessionLocal, engine
from app.models import AppUser, Base
from app.security import hash_password

DEMO_USERS = (
    {
        "username": "balttrade",
        "password": "client123",
        "role": "client",
        "client_id": 1,
    },
    {
        "username": "dispatcher",
        "password": "dispatcher123",
        "role": "dispatcher",
        "client_id": None,
    },
    {
        "username": "admin",
        "password": "admin123",
        "role": "admin",
        "client_id": None,
    },
)


def seed_app_users() -> None:
    Base.metadata.create_all(bind=engine, tables=[AppUser.__table__])
    db = SessionLocal()
    try:
        for item in DEMO_USERS:
            exists = db.query(AppUser).filter(AppUser.username == item["username"]).first()
            if exists:
                continue
            db.add(
                AppUser(
                    username=item["username"],
                    password_hash=hash_password(item["password"]),
                    role=item["role"],
                    client_id=item["client_id"],
                )
            )
        db.commit()
    finally:
        db.close()
