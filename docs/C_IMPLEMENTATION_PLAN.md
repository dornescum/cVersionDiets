# C Implementation Plan

Diet API implemented in C for maximum performance benchmarking.

## Tech Stack

| Component | Library | Why |
|-----------|---------|-----|
| HTTP Server | libmicrohttpd | Lightweight, well-documented, GNU project |
| MySQL Client | libmysqlclient | Official MySQL C connector |
| JSON | cJSON | Simple, single-file, MIT license |
| Build | CMake or Makefile | Cross-platform |

## Dependencies Installation

### macOS (Development)

```bash
# Using Homebrew
brew install libmicrohttpd
brew install mysql-client
brew install cmake

# cJSON (header-only, include in project)
# Or: brew install cjson
```

### Ubuntu 24 (VPS Production)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    libmicrohttpd-dev \
    libmysqlclient-dev \
    libcjson-dev
```

## Project Structure

```
c-diet-api/
├── CMakeLists.txt          # Build configuration
├── Makefile                 # Alternative simple build
├── Dockerfile
├── .env.example
├── README.md
│
├── include/
│   ├── config.h             # Configuration (env vars, constants)
│   ├── db.h                 # Database connection & queries
│   ├── routes.h             # Route handlers
│   ├── json_helpers.h       # JSON serialization helpers
│   └── http_helpers.h       # HTTP response helpers
│
├── src/
│   ├── main.c               # Entry point, server setup
│   ├── config.c             # Load .env, configuration
│   ├── db.c                 # MySQL connection pool, queries
│   ├── routes.c             # All route handlers
│   ├── routes_categories.c  # /api/categories handlers
│   ├── routes_foods.c       # /api/foods handlers
│   ├── routes_templates.c   # /api/templates handlers
│   ├── routes_benchmark.c   # /api/benchmark handlers
│   ├── json_helpers.c       # JSON building utilities
│   └── http_helpers.c       # Response utilities
│
└── vendor/
    └── cJSON/               # cJSON library (if not system-installed)
        ├── cJSON.h
        └── cJSON.c
```

## Endpoints to Implement

### Priority 1 - Core (for benchmarking)

| Endpoint | Method | Handler Function |
|----------|--------|------------------|
| `/health` | GET | `handle_health()` |
| `/api/categories` | GET | `handle_list_categories()` |
| `/api/categories/{id}` | GET | `handle_get_category()` |
| `/api/foods` | GET | `handle_list_foods()` |
| `/api/foods/{id}` | GET | `handle_get_food()` |
| `/api/templates/{id}/full` | GET | `handle_get_template_full()` |
| `/api/benchmark/bulk-insert` | POST | `handle_bulk_insert()` |

### Priority 2 - Complete API

| Endpoint | Method | Handler Function |
|----------|--------|------------------|
| `/api/categories` | POST | `handle_create_category()` |
| `/api/foods` | POST | `handle_create_food()` |
| `/api/templates` | GET | `handle_list_templates()` |
| `/api/templates/{id}` | GET | `handle_get_template()` |
| `/api/templates` | POST | `handle_create_template()` |

## Code Snippets

### main.c - Server Setup

```c
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "db.h"
#include "routes.h"

#define DEFAULT_PORT 8080

static enum MHD_Result
request_handler(void *cls,
                struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **con_cls)
{
    // Route matching
    if (strcmp(url, "/health") == 0 && strcmp(method, "GET") == 0) {
        return handle_health(connection);
    }

    if (strcmp(url, "/api/categories") == 0 && strcmp(method, "GET") == 0) {
        return handle_list_categories(connection);
    }

    // ... more routes

    // 404 Not Found
    return send_json_response(connection, 404, "{\"error\": \"Not found\"}");
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    struct MHD_Daemon *daemon;

    // Load configuration
    load_config();

    // Initialize database connection pool
    if (db_init() != 0) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }

    // Start HTTP server
    daemon = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,  // Threading model
        port,
        NULL, NULL,                      // Accept policy
        &request_handler, NULL,          // Handler
        MHD_OPTION_END
    );

    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", port);
    printf("Press Enter to stop...\n");
    getchar();

    MHD_stop_daemon(daemon);
    db_cleanup();

    return 0;
}
```

### db.c - Database Connection

```c
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "db.h"

static MYSQL *db_conn = NULL;

int db_init(void)
{
    db_conn = mysql_init(NULL);
    if (db_conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return -1;
    }

    if (mysql_real_connect(db_conn,
                           config.db_host,
                           config.db_user,
                           config.db_password,
                           config.db_name,
                           config.db_port,
                           NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n",
                mysql_error(db_conn));
        mysql_close(db_conn);
        return -1;
    }

    return 0;
}

MYSQL_RES *db_query(const char *query)
{
    if (mysql_query(db_conn, query) != 0) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(db_conn));
        return NULL;
    }
    return mysql_store_result(db_conn);
}

void db_cleanup(void)
{
    if (db_conn != NULL) {
        mysql_close(db_conn);
        db_conn = NULL;
    }
}
```

### routes_categories.c - Example Handler

```c
#include <microhttpd.h>
#include <cjson/cJSON.h>
#include "db.h"
#include "http_helpers.h"

enum MHD_Result handle_list_categories(struct MHD_Connection *connection)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *categories, *item;
    char *json_str;
    enum MHD_Result ret;

    // Query database
    result = db_query(
        "SELECT id, name, icon, color, sort_order "
        "FROM food_categories ORDER BY sort_order"
    );

    if (result == NULL) {
        return send_json_response(connection, 500,
            "{\"success\": false, \"error\": \"Database error\"}");
    }

    // Build JSON response
    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    categories = cJSON_AddArrayToObject(root, "categories");

    while ((row = mysql_fetch_row(result)) != NULL) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", atoi(row[0]));
        cJSON_AddStringToObject(item, "name", row[1] ? row[1] : "");
        cJSON_AddStringToObject(item, "icon", row[2] ? row[2] : "");
        cJSON_AddStringToObject(item, "color", row[3] ? row[3] : "");
        cJSON_AddNumberToObject(item, "sort_order", atoi(row[4]));
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

### http_helpers.c - Response Utilities

```c
#include <microhttpd.h>
#include <string.h>

enum MHD_Result send_json_response(
    struct MHD_Connection *connection,
    int status_code,
    const char *json_body)
{
    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(
        strlen(json_body),
        (void *)json_body,
        MHD_RESPMEM_MUST_COPY
    );

    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");

    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(diet_api_c VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(MYSQL REQUIRED mysqlclient)
pkg_check_modules(CJSON cjson)

# Source files
set(SOURCES
    src/main.c
    src/config.c
    src/db.c
    src/routes.c
    src/routes_categories.c
    src/routes_foods.c
    src/routes_templates.c
    src/routes_benchmark.c
    src/json_helpers.c
    src/http_helpers.c
)

# Include cJSON from vendor if not system-installed
if(NOT CJSON_FOUND)
    list(APPEND SOURCES vendor/cJSON/cJSON.c)
    include_directories(vendor/cJSON)
endif()

# Create executable
add_executable(diet_api ${SOURCES})

# Include directories
target_include_directories(diet_api PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${MICROHTTPD_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(diet_api
    ${MICROHTTPD_LIBRARIES}
    ${MYSQL_LIBRARIES}
)

if(CJSON_FOUND)
    target_link_libraries(diet_api ${CJSON_LIBRARIES})
endif()
```

## Simple Makefile (Alternative)

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include
LDFLAGS = -lmicrohttpd -lmysqlclient -lcjson

# macOS specific paths (Homebrew)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -I/opt/homebrew/include -I/opt/homebrew/opt/mysql-client/include
    LDFLAGS += -L/opt/homebrew/lib -L/opt/homebrew/opt/mysql-client/lib
endif

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/diet_api

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
 $(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
 $(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
 mkdir -p $@

clean:
 rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean
```

## Dockerfile

```dockerfile
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libmicrohttpd-dev \
    libmysqlclient-dev \
    libcjson-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make

# Runtime image
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libmicrohttpd12 \
    libmysqlclient21 \
    libcjson1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/diet_api .

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=3s \
    CMD curl -f http://localhost:8080/health || exit 1

CMD ["./diet_api"]
```

## Development Steps

### Phase 1: Setup & Health Check

- [ ] Create project structure
- [ ] Setup CMakeLists.txt / Makefile
- [ ] Implement main.c with basic server
- [ ] Implement /health endpoint
- [ ] Test on macOS

### Phase 2: Database

- [ ] Implement db.c with connection
- [ ] Load config from environment
- [ ] Test MySQL connection

### Phase 3: Core Endpoints

- [ ] GET /api/categories
- [ ] GET /api/categories/{id}
- [ ] GET /api/foods (with query params)
- [ ] GET /api/foods/{id}

### Phase 4: Complex Endpoints

- [ ] GET /api/templates/{id}/full (nested JSON)
- [ ] POST /api/benchmark/bulk-insert

### Phase 5: Docker & Deploy

- [ ] Create Dockerfile
- [ ] Test on Ubuntu 24 (Docker or VM)
- [ ] Run benchmarks

## Challenges to Consider

1. **Memory Management** - Free all allocated memory, avoid leaks
2. **SQL Injection** - Use prepared statements or escape strings
3. **Connection Pooling** - libmicrohttpd is multi-threaded, need thread-safe DB access
4. **URL Parsing** - Extract path parameters like `/api/foods/{id}`
5. **Query String Parsing** - Parse `?category_id=1&search=chicken`

## Performance Tips

1. Use `MHD_USE_EPOLL` on Linux for better performance
2. Pre-allocate JSON buffers where possible
3. Use connection pooling for MySQL
4. Consider using prepared statements
5. Compile with `-O3` for production

 <!-- To run it:
  ./run.sh              # Loads .env and runs

# or directly

  ./bin/diet_api        # Uses default config -->
