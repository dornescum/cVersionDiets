/**
 * @file config.h
 * @brief Application configuration management.
 *
 * Handles loading configuration from environment variables
 * for database connection and server settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief Application configuration structure.
 *
 * Holds all configuration values loaded from environment variables.
 * Memory for string fields is dynamically allocated.
 */
typedef struct {
    char *db_host;      /**< MySQL server hostname (env: DB_HOST) */
    char *db_user;      /**< MySQL username (env: DB_USER) */
    char *db_password;  /**< MySQL password (env: DB_PASSWORD) */
    char *db_name;      /**< MySQL database name (env: DB_NAME) */
    int db_port;        /**< MySQL server port (env: DB_PORT, default: 3306) */
    int server_port;    /**< HTTP server port (env: PORT, default: 8080) */
} Config;

/** @brief Global configuration instance */
extern Config config;

/**
 * @brief Loads configuration from environment variables.
 *
 * Reads environment variables and populates the global config struct.
 * Uses default values if variables are not set.
 *
 * @return 0 on success, -1 on failure
 */
int load_config(void);

/**
 * @brief Frees allocated configuration memory.
 *
 * Must be called before program exit to prevent memory leaks.
 */
void free_config(void);

#endif
