import { useState } from "react";
import { columnLabels, formColumns, toFormValues } from "../utils/labels";
import { Trash, Pencil, EllipsisVertical } from "lucide-react";
import { RowDialog } from "./row-dialog";

interface TableLayoutProps {
  tableName: string;
  columns: string[];
  items: Record<string, unknown>[];
  canWrite: boolean;
  canDelete: boolean;
  role?: string;
  onEdit: (rowId: string, data: Record<string, string>) => Promise<void>;
  onDelete: (item: Record<string, unknown>) => void;
}

export function TableLayout({
  tableName,
  columns,
  items,
  canWrite,
  canDelete,
  role,
  onEdit,
  onDelete,
}: TableLayoutProps) {
  const [editDialogOpen, setEditDialogOpen] = useState(false);
  const [editItem, setEditItem] = useState<Record<string, unknown> | null>(
    null,
  );

  if (items.length === 0) {
    return <p className="mt-4">Нет записей</p>;
  }

  return (
    <div className="mt-4 overflow-hidden rounded-sm border border-border">
      <table className="table-auto w-full">
        <thead>
          <tr className="border-b border-border">
            {columns.map((key) => (
              <th className="bg-accent-bg text-accent p-1.5" key={key}>
                {columnLabels[key] ?? key}
              </th>
            ))}
            {(canDelete || canWrite) && (
              <th className="bg-accent-bg text-accent p-1.5 w-10">
                <div className="flex items-center justify-center">
                  <EllipsisVertical className="w-4 h-4" />
                </div>
              </th>
            )}
          </tr>
        </thead>
        <tbody>
          {items.map((item: Record<string, unknown>, index: number) => (
            <tr
              key={index}
              className={index % 2 === 1 ? "bg-secondary-container/40" : ""}
            >
              {Object.entries(item).map(([column, value]) => (
                <td
                  className="p-1.5 border-r border-border last:border-r-0"
                  key={column}
                >
                  {value as string}
                </td>
              ))}
              {(canDelete || canWrite) && (
                <td className="w-fit">
                  <div className="flex items-center justify-center gap-2 p-1.5">
                    {canWrite && (
                      <button
                        className="hover:text-primary cursor-pointer"
                        onClick={() => {
                          setEditDialogOpen(true);
                          setEditItem(item);
                        }}
                      >
                        <Pencil className="w-4 h-4" />
                      </button>
                    )}
                    {canDelete && (
                      <button
                        className="hover:text-error cursor-pointer"
                        onClick={() => onDelete(item)}
                      >
                        <Trash className="w-4 h-4" />
                      </button>
                    )}
                  </div>
                </td>
              )}
            </tr>
          ))}
        </tbody>
      </table>
      {editDialogOpen && editItem && (
        <RowDialog
          mode="edit"
          tableName={tableName}
          columns={formColumns(columns, tableName, role)}
          initialData={toFormValues(editItem)}
          onClose={() => {
            setEditDialogOpen(false);
            setEditItem(null);
          }}
          onSubmit={async (data) => onEdit(String(editItem.id), data)}
        />
      )}
    </div>
  );
}
