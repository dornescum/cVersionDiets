/**
 * @file config.c
 * @brief Implementation of configuration loading from environment variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

/** @brief Global configuration instance */
Config config = {0};

/**
 * @brief Gets an environment variable or returns a default value.
 *
 * @param name Environment variable name
 * @param default_val Default value if variable is not set or empty
 * @return Newly allocated string (caller must free)
 */
static char *get_env_or_default(const char *name, const char *default_val) {
    const char *val = getenv(name);
    if (val == NULL || val[0] == '\0') {
        val = default_val;
    }
    return strdup(val);
}

/**
 * @brief Gets an environment variable as integer or returns a default.
 *
 * @param name Environment variable name
 * @param default_val Default value if variable is not set or empty
 * @return Integer value of environment variable or default
 */
static int get_env_int_or_default(const char *name, int default_val) {
    const char *val = getenv(name);
    if (val == NULL || val[0] == '\0') {
        return default_val;
    }
    return atoi(val);
}

int load_config(void) {
    config.db_host = get_env_or_default("DB_HOST", "localhost");
    config.db_user = get_env_or_default("DB_USER", "root");
    config.db_password = get_env_or_default("DB_PASSWORD", "");
    config.db_name = get_env_or_default("DB_NAME", "diet_api");
    config.db_port = get_env_int_or_default("DB_PORT", 3306);
    config.server_port = get_env_int_or_default("PORT", 8080);

    return 0;
}

void free_config(void) {
    free(config.db_host);
    free(config.db_user);
    free(config.db_password);
    free(config.db_name);
    config.db_host = NULL;
    config.db_user = NULL;
    config.db_password = NULL;
    config.db_name = NULL;
}
