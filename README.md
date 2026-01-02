# Diet API - C Implementation

High-performance REST API implementation in C for benchmarking against Python/FastAPI.

## Quick Start

```bash
# Build
make

# Run server
./run.sh

# Run benchmarks (in another terminal)
./benchmarks/run-benchmark.sh http://localhost:8085

# Analyze results
python analysis/analyze.py
```

## Dependencies (macOS)

```bash
brew install libmicrohttpd mysql-client cjson
```

## Project Structure

```
├── src/           # C source files
├── include/       # Header files
├── bin/           # Compiled binary
├── obj/           # Object files
├── benchmarks/    # k6 load testing scripts
├── analysis/      # Python analysis scripts
└── docs/          # Documentation
```

## Problems Encountered & Solutions

### 1. MySQL Connection: Socket vs TCP

**Problem:** Server failed to connect to MySQL with error:

```
Can't connect to local MySQL server through socket '/tmp/mysql.sock'
```

**Cause:** When `DB_HOST=localhost`, MySQL client uses Unix socket. Docker MySQL only exposes TCP port.

**Solution:** Use IP address instead of hostname:

```bash
# .env
DB_HOST=127.0.0.1  # Forces TCP connection
# NOT: DB_HOST=localhost  # Uses Unix socket
```

### 2. Database Schema Mismatch

**Problem:** Queries failed with "Table not found" and "Unknown column" errors.

**Cause:** Code assumed different table/column names than actual schema.

**Solution:** Updated queries to match actual schema:

```c
// Wrong:
"SELECT * FROM foods"
"SELECT calories, protein FROM ..."

// Correct:
"SELECT * FROM food_items"
"SELECT calories_per_100g, protein_per_100g FROM ..."
```

**Lesson:** Always verify database schema before writing queries.

### 3. Thread Safety - 99% Error Rate Under Load

**Problem:** Benchmark showed 99-100% error rate on all endpoints despite server responding.

**Cause:** libmicrohttpd uses thread-per-connection model. Single shared MySQL connection was being accessed by multiple threads simultaneously, causing race conditions and query failures.

**Symptoms:**

- Server responds (low latency)
- But returns 500 errors
- Works fine with single requests, fails under load

**Solution:** Added mutex protection around database operations:

```c
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

MYSQL_RES *db_query(const char *query) {
    pthread_mutex_lock(&db_mutex);
    // ... execute query ...
    pthread_mutex_unlock(&db_mutex);
    return result;
}
```

### 4. Port Already in Use

**Problem:** Server failed to start with "Failed to start HTTP server"

**Cause:** Previous server instance still running or port not released.

**Solution:**

```bash
# Kill process on port 8085
lsof -ti :8085 | xargs kill -9

# Then start server
./run.sh
```

### 5. Connection Pooling Bottleneck

**Problem:** C performs worse than Python on write-heavy workloads (bulk-insert).

**Cause:** Single mutex-protected connection serializes ALL database operations. Under concurrent load, threads queue up waiting for the mutex.

**Impact:**

- C bulk-insert: 78.79ms avg (worse)
- Python bulk-insert: 62.60ms avg (better - has connection pooling)

**Current Workaround:** Accept the limitation for read-heavy workloads where C still excels.

**Proper Solution (not implemented):** Implement connection pooling:

```c
// Pseudocode for connection pool
#define POOL_SIZE 10
static MYSQL *connection_pool[POOL_SIZE];
static pthread_mutex_t pool_mutex;
static sem_t pool_semaphore;

MYSQL *pool_get_connection(void) {
    sem_wait(&pool_semaphore);
    pthread_mutex_lock(&pool_mutex);
    // Find and return available connection
    pthread_mutex_unlock(&pool_mutex);
}

void pool_release_connection(MYSQL *conn) {
    pthread_mutex_lock(&pool_mutex);
    // Mark connection as available
    pthread_mutex_unlock(&pool_mutex);
    sem_post(&pool_semaphore);
}
```

### 6. Nested Queries Performance

**Problem:** template-full endpoint has high latency (580ms avg) due to multiple database round-trips.

**Cause:** Current implementation:

1. Query template (1 query)
2. Query days (1 query)
3. For each day, query meals (N queries)
4. For each meal, query items (M queries)

Total: 1 + 1 + N + (N * M) queries per request.

**Solution (not implemented):** Use JOINs to fetch all data in 1-3 queries:

```sql
SELECT t.*, d.*, m.*, mi.*, f.*
FROM diet_templates t
JOIN diet_days d ON d.template_id = t.id
JOIN diet_meals m ON m.day_id = d.id
JOIN diet_meal_items mi ON mi.meal_id = m.id
JOIN food_items f ON f.id = mi.food_item_id
WHERE t.id = ?
ORDER BY d.day_number, m.meal_order, mi.sort_order
```

## Benchmark Results

See [docs/BENCHMARK_COMPARISON.md](docs/BENCHMARK_COMPARISON.md) for detailed comparison with Python/FastAPI.

**Summary:**

| Endpoint | C vs Python |
|----------|-------------|
| categories | C is 3.2x faster |
| foods-list | C is 3.7x faster |
| bulk-insert | Python is 1.3x faster |
| template-full | C handles 3.2x more requests |

## Configuration

Create `.env` file:

```bash
DB_HOST=127.0.0.1
DB_USER
DB_PASSWORD
DB_NAME
DB_PORT=3306
PORT=8085
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /health | Health check |
| GET | /api/categories | List all categories |
| GET | /api/categories/{id} | Get category by ID |
| GET | /api/foods | List foods (with filters) |
| GET | /api/foods/{id} | Get food by ID |
| GET | /api/templates/{id}/full | Get full template with nested data |
| POST | /api/benchmark/bulk-insert | Bulk insert meal items |

## Documentation

- [C Project Structure](docs/C_PROJECT_STRUCTURE.md)
- [C Syntax Guide](docs/C_SYNTAX_GUIDE.md)
- [Benchmark Comparison](docs/BENCHMARK_COMPARISON.md)

./benchmarks/run-benchmark.sh <http://127.0.0.1:8085>

./run.sh &

./benchmarks/run-benchmark.sh <http://localhost:8085>
  python analysis/analyze.py
