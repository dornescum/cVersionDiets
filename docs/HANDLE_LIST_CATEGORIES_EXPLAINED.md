# handle_list_categories() Function Explained

## The Code

```c
enum MHD_Result handle_list_categories(struct MHD_Connection *connection) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *categories, *item;
    char *json_str;
    enum MHD_Result ret;

    result = db_query(
        "SELECT id, name, icon, color, sort_order "
        "FROM food_categories ORDER BY sort_order"
    );

    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    categories = cJSON_AddArrayToObject(root, "categories");

    while ((row = mysql_fetch_row(result)) != NULL) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", atoi(row[0]));
        cJSON_AddStringToObject(item, "name", row[1] ? row[1] : "");
        cJSON_AddStringToObject(item, "icon", row[2] ? row[2] : "");
        cJSON_AddStringToObject(item, "color", row[3] ? row[3] : "");
        cJSON_AddNumberToObject(item, "sort_order", row[4] ? atoi(row[4]) : 0);
        cJSON_AddItemToArray(categories, item);
    }

    cJSON_AddNumberToObject(root, "count", cJSON_GetArraySize(categories));

    mysql_free_result(result);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}
```

---

## Line-by-Line Breakdown

### Variable Declarations

```c
MYSQL_RES *result;      // Pointer to query result set
MYSQL_ROW row;          // One row of data (array of strings)
cJSON *root, *categories, *item;  // JSON objects
char *json_str;         // Final JSON string
enum MHD_Result ret;    // Return value
```

---

### Database Query

```c
result = db_query(
    "SELECT id, name, icon, color, sort_order "
    "FROM food_categories ORDER BY sort_order"
);
```
Execute SQL, get result set back.

```c
if (result == NULL) {
    return send_error_response(connection, 500, "Database error");
}
```
If query failed, return 500 error immediately.

---

### Build JSON Structure

```c
root = cJSON_CreateObject();                             // {}
cJSON_AddBoolToObject(root, "success", 1);               // {"success": true}
categories = cJSON_AddArrayToObject(root, "categories"); // {"success": true, "categories": []}
```

---

### Loop Through Rows

```c
while ((row = mysql_fetch_row(result)) != NULL) {
```
Fetch next row. `row` is an array of strings: `row[0]`, `row[1]`, etc.

```c
    item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "id", atoi(row[0]));      // row[0] = "1" → 1
    cJSON_AddStringToObject(item, "name", row[1] ? row[1] : "");
```

| Expression | Meaning |
|------------|---------|
| `row[0]` | First column (id) as string `"1"` |
| `atoi(row[0])` | Convert string to int: `"1"` → `1` |
| `row[1] ? row[1] : ""` | If NULL, use empty string (like `row[1] ?? ""` in TS) |

```c
    cJSON_AddItemToArray(categories, item);  // Push to array
}
```

---

### Add Count & Cleanup

```c
cJSON_AddNumberToObject(root, "count", cJSON_GetArraySize(categories));
```
Add `"count": 13` (array length)

```c
mysql_free_result(result);  // Free database result memory
```

---

### Send Response & Cleanup

```c
json_str = cJSON_PrintUnformatted(root);
ret = send_json_response(connection, 200, json_str);

free(json_str);
cJSON_Delete(root);  // Also deletes categories & all items inside

return ret;
```

---

## Final JSON Output

```json
{
  "success": true,
  "categories": [
    {"id": 1, "name": "Fruits", "icon": "fa-apple", "color": "#28a745", "sort_order": 1},
    {"id": 2, "name": "Vegetables", "icon": "fa-carrot", "color": "#fd7e14", "sort_order": 2}
  ],
  "count": 2
}
```

---

## TypeScript Equivalent

```typescript
async function handleListCategories(connection: Connection): Promise<Result> {
  const result = await db.query(
    "SELECT id, name, icon, color, sort_order FROM food_categories ORDER BY sort_order"
  );

  if (!result) {
    return sendErrorResponse(connection, 500, "Database error");
  }

  const categories = result.map(row => ({
    id: row.id,
    name: row.name ?? "",
    icon: row.icon ?? "",
    color: row.color ?? "",
    sort_order: row.sort_order ?? 0
  }));

  const response = {
    success: true,
    categories,
    count: categories.length
  };

  return sendJsonResponse(connection, 200, JSON.stringify(response));
}
```

---

## Key Concepts

| C | TypeScript | Description |
|---|------------|-------------|
| `MYSQL_RES *result` | `QueryResult` | Result set from database |
| `MYSQL_ROW row` | `row: Record` | Single row (array of strings in C) |
| `mysql_fetch_row(result)` | `result.next()` / iterator | Get next row |
| `atoi(row[0])` | `parseInt(row.id)` | String to integer |
| `row[1] ? row[1] : ""` | `row.name ?? ""` | Null coalescing |
| `mysql_free_result(result)` | (automatic) | Free memory |
| `cJSON_Delete(root)` | (automatic) | Free JSON tree |

---

## Memory Flow

```
1. db_query()        → allocates MYSQL_RES
2. cJSON_Create*()   → allocates JSON objects
3. cJSON_Print*()    → allocates string

4. mysql_free_result() → frees MYSQL_RES
5. free(json_str)      → frees string
6. cJSON_Delete(root)  → frees ALL JSON (root + categories + items)
```

**Important:** `cJSON_Delete(root)` recursively frees everything attached to it.
