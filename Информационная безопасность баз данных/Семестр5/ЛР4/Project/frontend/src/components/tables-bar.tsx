import { tableLabels } from "../utils/labels";
import type { TableInfo, Table } from "../types/types";

interface TablesBarProps {
  tablesInfo: TableInfo[];
  selectedTable: Table | null;
  getTable: (tableName: string) => void;
}

export function TablesBar({
  tablesInfo,
  selectedTable,
  getTable,
}: TablesBarProps) {
  return (
    <ul className="mt-4 flex justify-center flex-wrap">
      {tablesInfo.map((table) => (
        <li
          key={table.name}
          className={`first:rounded-l-md last:rounded-r-md border-border border hover:text-primary transition-colors duration-200 ${selectedTable?.name === table.name ? "bg-secondary-container text-accent" : ""}`}
        >
          <button
            className="w-full py-1.5 px-3 cursor-pointer"
            onClick={() => getTable(table.name)}
          >
            {tableLabels[table.name]}
          </button>
        </li>
      ))}
    </ul>
  );
}
