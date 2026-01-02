# C vs Python Benchmark Comparison

Benchmark performed on macOS with MySQL 8.0 running in Docker.

## Test Environment
- **C Server:** libmicrohttpd + libmysqlclient + cJSON
- **Python Server:** FastAPI + asyncpg/aiomysql
- **Load Testing:** k6 with 50 VUs (virtual users)
- **Database:** MySQL 8.0 in Docker container

## Results Summary

| Test | C Avg | Python Avg | C P50 | Python P50 | Winner |
|------|-------|------------|-------|------------|--------|
| **categories** | 2.91ms | 9.18ms | 2.24ms | 7.05ms | **C (3.2x faster)** |
| **foods-list** | 2.89ms | 10.68ms | 2.22ms | 8.18ms | **C (3.7x faster)** |
| **bulk-insert** | 78.79ms | 62.60ms | 58.62ms | 64.15ms | **Python (1.3x faster)** |
| **template-full** | 580ms | 5699ms | 318ms | 101ms | **Mixed** |

## Detailed Results

### C Implementation

| Test | Requests | Avg (ms) | P50 (ms) | P95 (ms) | P99 (ms) | Errors |
|------|----------|----------|----------|----------|----------|--------|
| categories | 19,399 | 2.91 | 2.24 | 7.42 | 12.43 | 0.00% |
| foods-list | 19,327 | 2.89 | 2.22 | 7.66 | 11.50 | 0.00% |
| bulk-insert | 703 | 78.79 | 58.62 | 174.96 | 408.52 | 0.00% |
| template-full | 743 | 580.46 | 318.88 | 1900.61 | 2938.66 | 0.00% |

### Python (FastAPI) Implementation

| Test | Requests | Avg (ms) | P50 (ms) | P95 (ms) | P99 (ms) | Errors |
|------|----------|----------|----------|----------|----------|--------|
| categories | 18,329 | 9.18 | 7.05 | 19.78 | 42.80 | 0.00% |
| foods-list | 18,006 | 10.68 | 8.18 | 25.38 | 45.57 | 0.00% |
| bulk-insert | 723 | 62.60 | 64.15 | 83.68 | 109.56 | 0.00% |
| template-full | 232 | 5699.04 | 101.26 | 20814.09 | 22627.33 | 0.00% |

## Analysis

### Simple Reads (categories, foods-list)

**C is 3-4x faster.**

- Lower latency at all percentiles
- Higher throughput (5-7% more requests handled)
- Minimal overhead from libmicrohttpd's thread-per-connection model

### Bulk Writes (bulk-insert)

**Python is ~1.3x faster.**

The C implementation uses a single mutex-protected MySQL connection, which serializes all database operations. This creates a bottleneck under concurrent write load.

Python's async database drivers typically use connection pooling, allowing parallel database operations.

**C tail latency is worse:**
- C P99: 408ms
- Python P99: 109ms

### Complex Joins (template-full)

**Mixed results - depends on the metric:**

| Metric | C | Python | Winner |
|--------|---|--------|--------|
| Throughput | 743 req | 232 req | **C (3.2x)** |
| Avg Latency | 580ms | 5699ms | **C (10x)** |
| P50 Latency | 318ms | 101ms | **Python (3x)** |
| P99 Latency | 2938ms | 22627ms | **C (7.7x)** |

Python handles individual queries faster (better P50), but **collapses under load**:
- Python P95: 20,814ms (20+ seconds!)
- Python P99: 22,627ms

C maintains more consistent latency under high concurrency.

## Bottlenecks Identified

### C Implementation
1. **Single database connection with mutex** - serializes all DB operations
2. **No connection pooling** - limits concurrent query execution
3. **Nested queries in template-full** - multiple round-trips to DB

### Python Implementation
1. **GIL contention** - limits true parallelism
2. **High memory overhead** - each request has more overhead
3. **Tail latency under load** - degrades significantly at P95/P99

## Recommendations

### To Improve C Performance
1. Implement **connection pooling** (e.g., 10-20 connections)
2. Use **prepared statements** for repeated queries
3. Optimize template-full with **single JOIN query** instead of nested queries

### To Improve Python Performance
1. Increase **worker processes** (Uvicorn/Gunicorn)
2. Add **query caching** (Redis)
3. Optimize ORM queries or use raw SQL

## Conclusion

| Use Case | Recommendation |
|----------|----------------|
| High-concurrency reads | **C** |
| Low-latency individual requests | **Python** |
| Consistent tail latency | **C** |
| Rapid development | **Python** |
| Resource-constrained environment | **C** |

**C is better for high-concurrency read-heavy workloads** where consistent latency matters.

**Python is better for write-heavy workloads** (with connection pooling) and when development speed is prioritized.
