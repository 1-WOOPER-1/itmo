import type { TableInfo } from "../types/types";

const API = import.meta.env.VITE_API_BASE_URL;

async function readError(res: Response) {
  const error = await res.json().catch(() => null);
  const detail =
    typeof error?.detail === "string" ? error.detail : res.statusText;
  throw new Error(detail);
}

export async function fetchTables(token: string): Promise<TableInfo[]> {
  const res = await fetch(`${API}/tables`, {
    headers: {
      Authorization: `Bearer ${token}`,
    },
  });
  if (!res.ok) await readError(res);
  const data = await res.json();
  return data.tables;
}

export async function fetchTable(
  token: string,
  tableName: string,
): Promise<{ table: string; items: Record<string, unknown>[]; count: number }> {
  const res = await fetch(`${API}/tables/${tableName}`, {
    headers: {
      Authorization: `Bearer ${token}`,
    },
  });
  if (!res.ok) await readError(res);
  return res.json();
}

export async function createRow(
  token: string,
  tableName: string,
  payload: Record<string, string>,
) {
  const res = await fetch(`${API}/tables/${tableName}`, {
    method: "POST",
    headers: {
      Authorization: `Bearer ${token}`,
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
  if (!res.ok) await readError(res);
  return res.json();
}

export async function updateRow(
  token: string,
  tableName: string,
  rowId: string,
  payload: Record<string, string>,
): Promise<Record<string, unknown>> {
  const res = await fetch(`${API}/tables/${tableName}/${rowId}`, {
    method: "PATCH",
    headers: {
      Authorization: `Bearer ${token}`,
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
  if (!res.ok) await readError(res);
  return res.json();
}

export async function deleteRow(
  token: string,
  tableName: string,
  rowId: string,
): Promise<boolean> {
  const res = await fetch(`${API}/tables/${tableName}/${rowId}`, {
    method: "DELETE",
    headers: {
      Authorization: `Bearer ${token}`,
    },
  });
  if (!res.ok) await readError(res);
  return true;
}
