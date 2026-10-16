from fastapi import Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer
from sqlalchemy.orm import Session

from app.access import readable_tables
from app.config import settings
from app.db import get_db
from app.models import AppUser
from app.security import InvalidTokenError, decode_access_token

oauth2_scheme = OAuth2PasswordBearer(tokenUrl=f"{settings.API_V1_STR}/login")


def get_current_user(
    token: str = Depends(oauth2_scheme),
    db: Session = Depends(get_db),
) -> AppUser:
    credentials_error = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Не удалось проверить токен",
        headers={"WWW-Authenticate": "Bearer"},
    )
    try:
        payload = decode_access_token(token)
        username = payload.get("sub")
        if not username:
            raise credentials_error
    except InvalidTokenError as exc:
        raise credentials_error from exc

    user = db.query(AppUser).filter(AppUser.username == username).first()
    if user is None:
        raise credentials_error
    return user


def user_payload(user: AppUser) -> dict:
    return {
        "username": user.username,
        "role": user.role,
        "client_id": user.client_id,
        "tables": readable_tables(user.role),
    }
