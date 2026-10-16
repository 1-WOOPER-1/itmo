export interface User {
  username: string;
  role: string;
  client_id: number | null;
  tables: string[];
}

export interface TableInfo {
  can_delete: boolean;
  can_write: boolean;
  kind: string;
  name: string;
}

export interface Table extends TableInfo {
  count: number;
  items: Record<string, unknown>[];
}
