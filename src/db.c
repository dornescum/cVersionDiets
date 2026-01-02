/**
 * @file db.c
 * @brief MySQL database connection implementation.
 *
 * Uses a mutex to protect the single MySQL connection for thread-safety
 * with libmicrohttpd's thread-per-connection model.
 */

#include <mysql/mysql.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "db.h"

/** @brief Single MySQL connection handle */
static MYSQL *db_conn = NULL;

/** @brief Mutex to protect database access across threads */
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    MYSQL_RES *result;

    pthread_mutex_lock(&db_mutex);

    if (db_conn == NULL) {
        fprintf(stderr, "Database not connected\n");
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    if (mysql_query(db_conn, query) != 0) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(db_conn));
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    result = mysql_store_result(db_conn);
    pthread_mutex_unlock(&db_mutex);

    return result;
}

int db_execute(const char *query) {
    int affected_rows;

    pthread_mutex_lock(&db_mutex);

    if (db_conn == NULL) {
        fprintf(stderr, "Database not connected\n");
        pthread_mutex_unlock(&db_mutex);
        return -1;
    }

    if (mysql_query(db_conn, query) != 0) {
        fprintf(stderr, "Execute failed: %s\n", mysql_error(db_conn));
        pthread_mutex_unlock(&db_mutex);
        return -1;
    }

    affected_rows = (int)mysql_affected_rows(db_conn);
    pthread_mutex_unlock(&db_mutex);

    return affected_rows;
}

void db_cleanup(void) {
    if (db_conn != NULL) {
        mysql_close(db_conn);
        db_conn = NULL;
        printf("Database connection closed\n");
    }
}
