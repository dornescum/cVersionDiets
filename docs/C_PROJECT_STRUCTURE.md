# C Project Structure

## Directory Layout

```
project/
├── include/    # Header files (.h) - declarations
├── src/        # Source files (.c) - implementations
├── obj/        # Object files (.o) - compiled intermediates
├── bin/        # Executable binary
└── docs/       # Documentation
```

## `/include/` - Header Files (.h)

**Purpose:** Declarations, interfaces, "what exists"

```c
// db.h - tells other files "these functions exist"
int db_init(void);                        // Function declaration
MYSQL_RES *db_query(const char *query);
extern Config config;                     // Global variable declaration
```

**Contains:**
- Function **declarations** (signatures only, no code)
- Type definitions (`typedef struct {...} Config;`)
- Constants (`#define PORT 8080`)
- `extern` variable declarations

**Why separate?** So multiple `.c` files can `#include` the same header and know what functions/types exist.

---

## `/src/` - Source Files (.c)

**Purpose:** Implementations, "how it works"

```c
// db.c - the actual code
int db_init(void) {
    db_conn = mysql_init(NULL);  // Actual implementation
    ...
}
```

**Contains:**
- Function **definitions** (actual code)
- `static` variables (private to that file)
- `#include` headers it needs

---

## `/obj/` - Object Files (.o)

**Purpose:** Compiled binary files (intermediate step)

```
src/db.c   --[compile]--> obj/db.o   --[link]--> bin/diet_api
src/main.c --[compile]--> obj/main.o ----/
```

Each `.c` compiles to `.o`, then all `.o` files link together into the final executable.

---

## Build Flow

```
User code                 Compilation
─────────────────────────────────────────
include/db.h  ←──┐
                 │ #include
src/db.c ────────┘ ──compile──► obj/db.o ─┐
src/main.c ──────────compile──► obj/main.o├──link──► bin/diet_api
src/routes.c ────────compile──► obj/routes.o┘
```

**Benefits:**
- **Faster rebuilds:** Change one `.c` → recompile only that `.o`
- **Separation:** Headers = public API, Source = private implementation
- **Cleaner:** `.o` files are generated, easy to `make clean`
