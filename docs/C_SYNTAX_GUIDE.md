# C Syntax Guide (for TypeScript developers)

## Function Declaration

```c
enum MHD_Result handle_health(struct MHD_Connection *connection);
```

| Part | Meaning |
|------|---------|
| `enum MHD_Result` | Return type (like TypeScript's return type) |
| `handle_health` | Function name |
| `struct MHD_Connection *connection` | Parameter (pointer to a connection object) |

---

## `enum` in C vs TypeScript

```typescript
// TypeScript
enum Result {
  YES = 1,
  NO = 0
}

function handleHealth(): Result {
  return Result.YES;
}
```

```c
// C - almost the same!
enum MHD_Result {
  MHD_YES = 1,
  MHD_NO = 0
};

enum MHD_Result handle_health(...) {
  return MHD_YES;
}
```

**Same concept:** Named constants instead of magic numbers.

---

## `struct` = TypeScript `interface`

```typescript
// TypeScript
interface Connection {
  ip: string;
  port: number;
}

function handle(conn: Connection) { }
```

```c
// C
struct MHD_Connection {
  char *ip;
  int port;
};

void handle(struct MHD_Connection *conn) { }
```

---

## The `*` (pointer)

```c
struct MHD_Connection *connection
//                    ^ pointer = "reference to" (not a copy)
```

Like TypeScript objects, passed by reference - the function can access/modify the original, not a copy.

---

## Quick Reference

| TypeScript | C |
|------------|---|
| `interface` | `struct` |
| `enum` | `enum` |
| `: ReturnType` | Return type before function name |
| Object reference | `*` pointer |
| `null` | `NULL` |
| `string` | `char *` |
| `number` | `int`, `float`, `double` |
| `boolean` | `int` (0 = false, non-zero = true) |

---

## Example Comparison

```typescript
// TypeScript
interface Config {
  host: string;
  port: number;
}

function loadConfig(): Config {
  return { host: "localhost", port: 8080 };
}
```

```c
// C
struct Config {
  char *host;
  int port;
};

struct Config loadConfig(void) {
  struct Config cfg;
  cfg.host = "localhost";
  cfg.port = 8080;
  return cfg;
}
```
