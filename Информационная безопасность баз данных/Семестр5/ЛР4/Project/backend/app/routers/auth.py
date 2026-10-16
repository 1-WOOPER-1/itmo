from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordRequestForm
from sqlalchemy.orm import Session

from app.db import get_db
from app.deps import get_current_user, user_payload
from app.models import AppUser
from app.schemas import CurrentUser, Token
from app.security import create_access_token, verify_password

router = APIRouter(tags=["auth"])


@router.post("/login", response_model=Token)
def login(
    form_data: OAuth2PasswordRequestForm = Depends(),
    db: Session = Depends(get_db),
) -> Token:
    user = db.query(AppUser).filter(AppUser.username == form_data.username).first()
    if user is None or not verify_password(form_data.password, user.password_hash):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Неверный логин или пароль",
            headers={"WWW-Authenticate": "Bearer"},
        )
    token = create_access_token(username=user.username, role=user.role)
    return Token(access_token=token)


@router.get("/me", response_model=CurrentUser)
def me(user: AppUser = Depends(get_current_user)) -> dict:
    return user_payload(user)
