# handle_health() Function Explained

## The Code

```c
enum MHD_Result handle_health(struct MHD_Connection *connection) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "status", "ok");
    cJSON_AddStringToObject(root, "service", "diet-api-c");

    char *json_str = cJSON_PrintUnformatted(root);
    enum MHD_Result ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}
```

---

## Line-by-Line Breakdown

### Function Signature
```c
enum MHD_Result handle_health(struct MHD_Connection *connection) {
```
Function that returns `MHD_YES` or `MHD_NO`, receives the HTTP connection.

---

### Create JSON Object
```c
cJSON *root = cJSON_CreateObject();
```
Create empty JSON object `{}`
- `cJSON *` = pointer to a cJSON object
- Like TypeScript: `const root = {}`

---

### Add Key-Value Pairs
```c
cJSON_AddStringToObject(root, "status", "ok");
cJSON_AddStringToObject(root, "service", "diet-api-c");
```
Add key-value pairs
- Now: `{"status": "ok", "service": "diet-api-c"}`
- Like TypeScript: `root.status = "ok"`

---

### Convert to String
```c
char *json_str = cJSON_PrintUnformatted(root);
```
Convert object to JSON string
- Like TypeScript: `JSON.stringify(root)`
- Returns: `"{\"status\":\"ok\",\"service\":\"diet-api-c\"}"`

---

### Send HTTP Response
```c
enum MHD_Result ret = send_json_response(connection, 200, json_str);
```
Send HTTP response
- `connection` = who to send to
- `200` = HTTP status code (OK)
- `json_str` = response body

---

### Memory Cleanup
```c
free(json_str);
cJSON_Delete(root);
```
**Memory cleanup (no garbage collector in C!)**
- `free()` = release string memory
- `cJSON_Delete()` = release JSON object memory

In TypeScript, garbage collector does this automatically. In C, **you must free manually** or you get memory leaks.

---

### Return Result
```c
return ret;
```
Return `MHD_YES` (success) or `MHD_NO` (failure)

---

## TypeScript Equivalent

```typescript
function handleHealth(connection: Connection): Result {
  const root = {
    status: "ok",
    service: "diet-api-c"
  };

  const jsonStr = JSON.stringify(root);
  const ret = sendJsonResponse(connection, 200, jsonStr);

  // No free() needed - garbage collector handles it

  return ret;
}
```

---

## Key Differences: C vs TypeScript

| Aspect | C | TypeScript |
|--------|---|------------|
| JSON creation | `cJSON_CreateObject()` | `{}` literal |
| Add property | `cJSON_AddStringToObject(obj, key, val)` | `obj.key = val` |
| Stringify | `cJSON_PrintUnformatted(obj)` | `JSON.stringify(obj)` |
| Memory | Manual `free()` required | Automatic garbage collection |
| Strings | `char *` (pointer) | `string` type |
