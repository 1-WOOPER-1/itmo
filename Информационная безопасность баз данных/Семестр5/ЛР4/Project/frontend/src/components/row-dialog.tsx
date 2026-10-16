import { X } from "lucide-react";
import { useState } from "react";

interface RowDialogProps {
  mode: "create" | "edit";
  tableName: string;
  columns: string[];
  initialData?: Record<string, string>;
  onClose: () => void;
  onSubmit: (data: Record<string, string>) => Promise<void>;
}

export function RowDialog({
  mode,
  tableName,
  columns,
  initialData,
  onClose,
  onSubmit,
}: RowDialogProps) {
  const [error, setError] = useState<string | null>(null);

  async function handleSubmit(e: React.FormEvent<HTMLFormElement>) {
    e.preventDefault();
    const formData = new FormData(e.currentTarget);
    const data: Record<string, string> = {};
    formData.forEach((value, key) => {
      if (value === "" || key === "id" || key === "created_at") return;
      data[key] = String(value);
    });
    try {
      await onSubmit(data);
      onClose();
    } catch (error: unknown) {
      setError(
        error instanceof Error ? error.message : "Не удалось создать запись",
      );
    }
  }

  return (
    <div className="fixed inset-0 backdrop-blur-sm flex justify-center items-center">
      <div className="bg-surface rounded-md p-4 relative outline outline-border">
        <button
          className="absolute top-3 right-3 cursor-pointer"
          onClick={onClose}
        >
          <X className="w-6 h-6" onClick={onClose} />
        </button>
        <h3 className="max-w-73">
          {mode === "create" ? "Создать запись" : "Обновить запись"} в таблице{" "}
          {tableName}
        </h3>
        <form className="mt-4 flex flex-col gap-2" onSubmit={handleSubmit}>
          {columns.map((column) => (
            <div
              key={column}
              className="flex gap-2 items-center justify-between"
            >
              <label htmlFor={column}>{column}</label>
              <input
                className="p-2 rounded-md border border-border focus:outline-none focus:ring-2 focus:ring-primary"
                type="text"
                id={column}
                name={column}
                placeholder={column}
                defaultValue={initialData?.[column]}
              />
            </div>
          ))}
          {error && <p className="text-error">{error}</p>}
          <button
            className="flex items-center gap-2 bg-primary text-on-primary px-3 py-1.5 rounded-md hover:bg-primary/80 transition-colors duration-200 cursor-pointer"
            type="submit"
          >
            {mode === "create" ? "Создать" : "Обновить"}
          </button>
        </form>
      </div>
    </div>
  );
}
