import json
from json.decoder import JSONDecodeError
from logging import info
from pathlib import Path
from typing import Any, Dict, Optional

import httpx
from fastapi import FastAPI, Request, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel
from pydantic.functional_validators import InstanceOf


class ClientData(BaseModel):
    ipData: Optional[Dict[str, Any]] = None
    systemData: Optional[Dict[str, Any]] = None


app = FastAPI()
app.frontend("/", directory=".", fallback="index.html")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/")
async def read_root():
    return FileResponse("index.html")


@app.get("/api/ip")
async def get_ip(request: Request):
    forwarded_for = request.headers.get("x-forwarded-for")
    if forwarded_for:
        client_ip = forwarded_for.split(",")[0].strip()
    else:
        client_ip = request.client.host

    if client_ip in ("127.0.0.1", "localhost", "::1"):
        return {
            "ip": client_ip,
            "city": "Localhost",
            "country": "Local",
            "org": "Development",
        }

    async with httpx.AsyncClient() as client:
        response = await client.get(f"https://ipinfo.io/{client_ip}/json")
        return response.json()


@app.post("/api/client", status_code=status.HTTP_201_CREATED)
def save_client(request: Request, data: ClientData):
    print("IP Data:", data.ipData)
    print("System Data:", data.systemData)

    ip = (data.ipData.get("ip") if data.ipData else request.client.host) or "unknown"
    save_dir = Path(__file__).resolve().parent / "clients" / ip
    save_dir.mkdir(parents=True, exist_ok=True)

    info_path = save_dir / "info.json"

    if info_path.exists():
        try:
            with open(info_path) as f:
                old = json.load(f)
                if isinstance(old, dict):
                    old = [old]
        except JSONDecodeError:
            old = []
    else:
        old = []

    new_client = {"ip_data": data.ipData, "system_data": data.systemData}
    old.append(new_client)

    with open(save_dir / "info.json", "w", encoding="utf-8") as f:
        json.dump(
            old,
            f,
            ensure_ascii=False,
            indent=4,
        )

    return {"status": "success", "message": "Client data saved"}


app.mount("/", StaticFiles(directory="."), name="static")
