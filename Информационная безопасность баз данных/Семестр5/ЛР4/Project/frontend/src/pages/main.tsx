import { useEffect, useState, useCallback } from "react";
import { Navigate, useNavigate } from "react-router";
import { LogOut, Plus } from "lucide-react";
import { RowDialog } from "../components/row-dialog";
import { TableLayout } from "../components/table-layout";
import type { TableInfo, Table, User } from "../types/types";
import { TablesBar } from "../components/tables-bar";
import { fetchTables, fetchTable, createRow } from "../utils/tables";
import { updateRow, deleteRow } from "../utils/tables";
import { formColumns } from "../utils/labels";

const API = import.meta.env.VITE_API_BASE_URL;

export function Main() {
  const navigate = useNavigate();
  const [token, setToken] = useState(localStorage.getItem("token"));
  const [rowDialogOpen, setRowDialogOpen] = useState(false);
  const [user, setUser] = useState<User | null>(null);
  const [tablesInfo, setTablesInfo] = useState<TableInfo[]>([]);
  const [selectedTable, setSelectedTable] = useState<Table | null>(null);

  const getTable = useCallback(
    async (tableName: string) => {
      if (!token) return;
      const data = await fetchTable(token, tableName);
      const info = tablesInfo.find((table) => table.name === tableName);
      setSelectedTable({
        name: tableName,
        kind: info?.kind ?? "table",
        can_write: info?.can_write ?? false,
        can_delete: info?.can_delete ?? false,
        count: data.count,
        items: data.items,
      });
    },
    [token, tablesInfo],
  );

  const handleCreateSubmit = useCallback(
    async (data: Record<string, string>) => {
      if (!token || !selectedTable?.name) return;
      await createRow(token, selectedTable.name, data);
    },
    [token, selectedTable],
  );

  const handleEdit = useCallback(
    async (rowId: string, data: Record<string, string>) => {
      if (!token || !selectedTable?.name) return;
      await updateRow(token, selectedTable.name, rowId, data);
      await getTable(selectedTable.name);
    },
    [token, selectedTable, getTable],
  );

  const handleDelete = useCallback(
    async (item: Record<string, unknown>) => {
      if (!token || !selectedTable?.name) return;
      await deleteRow(token, selectedTable.name, String(item.id));
      setSelectedTable({
        ...selectedTable,
        items: selectedTable.items.filter((i) => i.id !== item.id),
      });
    },
    [token, selectedTable],
  );

  useEffect(() => {
    if (!token) return;
    const accessToken = token;

    let cancelled = false;

    async function load() {
      const res = await fetch(`${API}/me`, {
        headers: {
          Authorization: `Bearer ${accessToken}`,
        },
      });
      if (!res.ok) throw new Error(await res.text());
      const me = (await res.json()) as User;
      const tables = await fetchTables(accessToken);
      if (cancelled) return;
      setUser(me);
      setTablesInfo(tables);
    }

    void load();
    return () => {
      cancelled = true;
    };
  }, [token]);

  if (!token) {
    return <Navigate to="/login" replace />;
  }

  return (
    <main className="max-w-6xl mx-auto pt-16">
      <div className="flex justify-between items-center">
        <h2>Добро пожаловать, {user?.username}</h2>
        <button
          className="flex items-center gap-2 bg-secondary-container text-accent px-3 py-1.5 rounded-md hover:bg-secondary-container/80 transition-colors duration-200 cursor-pointer"
          onClick={() => {
            localStorage.removeItem("token");
            setToken(null);
            setUser(null);
            setSelectedTable(null);
            navigate("/login");
          }}
        >
          <LogOut className="w-4 h-4" />
          Выйти
        </button>
      </div>

      <button
        className="mt-4 flex items-center gap-2 bg-primary text-on-primary px-3 py-1.5 rounded-md hover:bg-primary/80 transition-colors duration-200 cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
        disabled={!selectedTable?.can_write}
        onClick={() => setRowDialogOpen(true)}
      >
        <Plus className="w-4 h-4" /> Создать запись
      </button>
      {rowDialogOpen && selectedTable?.can_write && (
        <RowDialog
          mode="create"
          tableName={selectedTable.name}
          columns={formColumns(
            Object.keys(selectedTable.items[0] ?? {}),
            selectedTable.name,
            user?.role,
          )}
          onClose={() => {
            setRowDialogOpen(false);
            getTable(selectedTable.name);
          }}
          onSubmit={handleCreateSubmit}
        />
      )}

      <TablesBar
        tablesInfo={tablesInfo}
        selectedTable={selectedTable}
        getTable={getTable}
      />

      {selectedTable && (
        <TableLayout
          tableName={selectedTable.name}
          columns={Object.keys(selectedTable.items[0] ?? {})}
          items={selectedTable.items}
          canWrite={selectedTable.can_write}
          canDelete={selectedTable.can_delete}
          role={user?.role}
          onEdit={handleEdit}
          onDelete={handleDelete}
        />
      )}
    </main>
  );
}
