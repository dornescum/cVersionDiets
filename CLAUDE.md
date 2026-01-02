# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Diet API implemented in C for maximum performance benchmarking. Uses libmicrohttpd for HTTP server, libmysqlclient for MySQL, and cJSON for JSON handling.

## Build Commands

### Using CMake
```bash
mkdir build && cd build
cmake ..
make
```

### Using Makefile
```bash
make          # Build the project
make clean    # Clean build artifacts
```

The binary outputs to `bin/diet_api`.

### Running
```bash
./bin/diet_api              # Runs on default port 8080
```

## Dependencies

### macOS (Homebrew)
```bash
brew install libmicrohttpd mysql-client cmake cjson
```

### Ubuntu 24
```bash
sudo apt install build-essential cmake libmicrohttpd-dev libmysqlclient-dev libcjson-dev
```

## Architecture

```
include/           # Header files (.h)
src/               # Implementation files (.c)
  main.c           # Entry point, HTTP server setup, route dispatch
  config.c         # Environment/configuration loading
  db.c             # MySQL connection and query functions
  routes_*.c       # Route handlers by resource (categories, foods, templates, benchmark)
  http_helpers.c   # HTTP response utilities
  json_helpers.c   # JSON serialization utilities
vendor/cJSON/      # cJSON library (if not system-installed)
```

## Key Patterns

- **Request handling**: `request_handler()` in main.c does URL/method matching and dispatches to handler functions
- **Database**: Single connection via `db_init()`/`db_query()`/`db_cleanup()` - needs thread-safe pooling for production
- **JSON responses**: Build with cJSON, serialize with `cJSON_PrintUnformatted()`, send via `send_json_response()`
- **Memory**: All cJSON objects and MySQL results must be freed after use

## API Endpoints

Core endpoints for benchmarking:
- `GET /health`
- `GET /api/categories`, `GET /api/categories/{id}`
- `GET /api/foods`, `GET /api/foods/{id}`
- `GET /api/templates/{id}/full`
- `POST /api/benchmark/bulk-insert`

## Compiler Flags

- Development: `-Wall -Wextra -O2`
- Production: `-O3`
- macOS needs: `-I/opt/homebrew/include -L/opt/homebrew/lib` for Homebrew paths
