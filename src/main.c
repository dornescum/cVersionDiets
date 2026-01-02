/**
 * @file main.c
 * @brief Diet API server entry point.
 *
 * Initializes configuration, database, and HTTP server.
 * Handles graceful shutdown on SIGINT/SIGTERM.
 */

#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "config.h"
#include "db.h"
#include "routes.h"
#include "http_helpers.h"

/** @brief Flag for graceful shutdown */
static volatile int running = 1;

/**
 * @brief Signal handler for graceful shutdown.
 *
 * Sets running flag to 0 to exit the main loop.
 *
 * @param sig Signal number (unused)
 */
static void handle_signal(int sig) {
    (void)sig;
    running = 0;
    printf("\nShutting down...\n");
}

/**
 * @brief Extracts numeric ID from URL path.
 *
 * Example: "/api/foods/123" with prefix "/api/foods/" returns 123
 *
 * @param url Full request URL
 * @param prefix URL prefix to strip (e.g., "/api/foods/")
 * @return Extracted ID, or -1 if URL doesn't match prefix
 */
static int extract_id_from_path(const char *url, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    if (strncmp(url, prefix, prefix_len) != 0) {
        return -1;
    }
    const char *id_str = url + prefix_len;
    if (*id_str == '\0') {
        return -1;
    }
    return atoi(id_str);
}

/**
 * @brief Main HTTP request handler callback.
 *
 * Routes incoming requests to appropriate handler functions
 * based on URL and HTTP method.
 *
 * @param cls Custom user data (unused)
 * @param connection MHD connection handle
 * @param url Request URL path
 * @param method HTTP method (GET, POST, etc.)
 * @param version HTTP version string (unused)
 * @param upload_data POST/PUT body data (unused)
 * @param upload_data_size Size of upload data (unused)
 * @param con_cls Connection-specific data (unused)
 * @return MHD_YES on success, MHD_NO on failure
 */
static enum MHD_Result request_handler(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls)
{
    (void)cls;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    (void)con_cls;

    /* Handle CORS preflight requests */
    if (strcmp(method, "OPTIONS") == 0) {
        return send_json_response(connection, 200, "{}");
    }

    /* Route: GET /health */
    if (strcmp(url, "/health") == 0 && strcmp(method, "GET") == 0) {
        return handle_health(connection);
    }

    /* Route: GET /api/categories */
    if (strcmp(url, "/api/categories") == 0 && strcmp(method, "GET") == 0) {
        return handle_list_categories(connection);
    }

    /* Route: GET /api/categories/{id} */
    if (strncmp(url, "/api/categories/", 16) == 0 && strcmp(method, "GET") == 0) {
        int id = extract_id_from_path(url, "/api/categories/");
        if (id > 0) {
            return handle_get_category(connection, id);
        }
    }

    /* Route: GET /api/foods */
    if (strcmp(url, "/api/foods") == 0 && strcmp(method, "GET") == 0) {
        return handle_list_foods(connection);
    }

    /* Route: GET /api/foods/{id} */
    if (strncmp(url, "/api/foods/", 11) == 0 && strcmp(method, "GET") == 0) {
        int id = extract_id_from_path(url, "/api/foods/");
        if (id > 0) {
            return handle_get_food(connection, id);
        }
    }

    /* 404 Not Found */
    return send_error_response(connection, 404, "Not found");
}

/**
 * @brief Application entry point.
 *
 * Initializes all components and starts the HTTP server.
 * Runs until SIGINT or SIGTERM is received.
 *
 * @param argc Argument count (unused)
 * @param argv Argument vector (unused)
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    struct MHD_Daemon *daemon;

    /* Setup signal handlers for graceful shutdown */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Load configuration from environment */
    if (load_config() != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        return 1;
    }

    printf("Diet API C Server\n");
    printf("=================\n");

    /* Initialize database connection */
    if (db_init() != 0) {
        fprintf(stderr, "Failed to initialize database (continuing without DB)\n");
    }

    /* Start HTTP server with thread-per-connection model */
    daemon = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD,
        config.server_port,
        NULL, NULL,
        &request_handler, NULL,
        MHD_OPTION_END
    );

    if (daemon == NULL) {
        fprintf(stderr, "Failed to start HTTP server\n");
        db_cleanup();
        free_config();
        return 1;
    }

    printf("Server running on http://localhost:%d\n", config.server_port);
    printf("Press Ctrl+C to stop\n\n");

    /* Main loop - wait for shutdown signal */
    while (running) {
        sleep(1);
    }

    /* Cleanup resources */
    MHD_stop_daemon(daemon);
    db_cleanup();
    free_config();

    printf("Server stopped\n");
    return 0;
}
