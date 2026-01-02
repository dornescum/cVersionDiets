/**
 * @file db.c
 * @brief MySQL database connection implementation.
 */

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "db.h"

/** @brief Single MySQL connection handle (not thread-safe) */
static MYSQL *db_conn = NULL;

int db_init(void) {
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
        db_conn = NULL;
        return -1;
    }

    printf("Connected to MySQL: %s@%s:%d/%s\n",
           config.db_user, config.db_host, config.db_port, config.db_name);

    return 0;
}

MYSQL *db_get_connection(void) {
    return db_conn;
}

MYSQL_RES *db_query(const char *query) {
    if (db_conn == NULL) {
        fprintf(stderr, "Database not connected\n");
        return NULL;
    }

    if (mysql_query(db_conn, query) != 0) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(db_conn));
        return NULL;
    }
    return mysql_store_result(db_conn);
}

void db_cleanup(void) {
    if (db_conn != NULL) {
        mysql_close(db_conn);
        db_conn = NULL;
        printf("Database connection closed\n");
    }
}
