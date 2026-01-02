/**
 * @file db.h
 * @brief MySQL database connection and query interface.
 *
 * Provides functions for connecting to MySQL, executing queries,
 * and managing the database connection lifecycle.
 *
 * @note Currently uses a single connection. For production with
 *       multi-threaded servers, implement connection pooling.
 */

#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

/**
 * @brief Initializes the database connection.
 *
 * Connects to MySQL using credentials from the global config.
 * Must be called after load_config().
 *
 * @return 0 on success, -1 on failure
 */
int db_init(void);

/**
 * @brief Closes the database connection and frees resources.
 *
 * Should be called before program exit.
 */
void db_cleanup(void);

/**
 * @brief Gets the active MySQL connection handle.
 *
 * @return Pointer to MYSQL connection, or NULL if not connected
 */
MYSQL *db_get_connection(void);

/**
 * @brief Executes a SQL query and returns the result set.
 *
 * @param query SQL query string to execute
 * @return MYSQL_RES pointer on success (caller must free with mysql_free_result),
 *         NULL on error
 *
 * @warning Query string is not escaped. Use mysql_real_escape_string()
 *          for user-provided values to prevent SQL injection.
 */
MYSQL_RES *db_query(const char *query);

#endif
