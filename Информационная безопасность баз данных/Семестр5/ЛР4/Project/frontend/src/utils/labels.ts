export const tableLabels: Record<string, string> = {
  clients: "Клиенты",
  ports: "Порты",
  ships: "Суда",
  containers: "Контейнеры",
  orders: "Заказы",
  voyages: "Рейсы",
  invoices: "Счета",
  order_containers: "Контейнеры в заказах",
  voyage_orders: "Заказы в рейсах",
  view_client_orders: "Реестр заказов и оплат",
  view_client_tracking: "Трекинг груза",
  view_voyage_manifest: "Грузовой манифест",
  view_container_safety_audit: "Контроль веса и пломб",
};

export const columnLabels: Record<string, string> = {
  id: "ID",
  name: "Название",
  email: "Email",
  phone: "Телефон",
  address: "Адрес",
  country: "Страна",
  dock_count: "Число причалов",
  max_draft: "Макс. осадка",
  imo_number: "IMO",
  deadweight: "Дедвейт",
  year_built: "Год постройки",
  flag_state: "Флаг",
  container_number: "Номер контейнера",
  type: "Тип",
  empty_weight: "Вес тары",
  capacity_volume: "Вместимость",
  client_id: "ID клиента",
  cargo_name: "Груз",
  cargo_weight: "Вес груза",
  cargo_volume: "Объём груза",
  total_price: "Стоимость",
  created_at: "Создан",
  ship_id: "ID судна",
  departure_port_id: "ID порта отправления",
  destination_port_id: "ID порта назначения",
  departure_date: "Отправление",
  arrival_date: "Прибытие",
  status: "Статус",
  order_id: "ID заказа",
  invoice_number: "Номер счёта",
  amount: "Сумма",
  payment_status: "Статус оплаты",
  issue_date: "Дата выставления",
  container_id: "ID контейнера",
  seal_number: "Пломба",
  gross_weight: "Брутто",
  voyage_id: "ID рейса",
  loading_date: "Погрузка",
  discharge_date: "Выгрузка",
  containers_count: "Число контейнеров",
  actual_gross_weight: "Фактический брутто",
  vessel_name: "Судно",
  port_of_loading: "Порт погрузки",
  port_of_discharge: "Порт выгрузки",
  voyage_status: "Статус рейса",
  ship_name: "Судно",
  ship_capacity_tons: "Вместимость, т",
  origin_port: "Порт отправления",
  destination_port: "Порт назначения",
  total_orders_assigned: "Заказов",
  total_cargo_tons: "Груз, т",
  container_type: "Тип контейнера",
  tare_weight: "Вес тары",
  net_cargo_weight: "Нетто",
  shipper_name: "Грузоотправитель",
};

export function formColumns(
  columns: string[],
  tableName: string,
  role?: string,
) {
  return columns.filter((column) => {
    if (column === "id" || column === "created_at") return false;
    if (role === "client" && tableName === "orders" && column === "client_id") {
      return false;
    }
    return true;
  });
}

export function toFormValues(
  item: Record<string, unknown>,
): Record<string, string> {
  return Object.fromEntries(
    Object.entries(item).map(([key, value]) => [
      key,
      value == null ? "" : String(value),
    ]),
  );
}
