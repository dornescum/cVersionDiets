/**
 * @file http_helpers.h
 * @brief HTTP response utilities for libmicrohttpd.
 *
 * Helper functions for sending JSON responses with proper
 * headers and CORS support.
 */

#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H

#include <microhttpd.h>

/**
 * @brief Sends a JSON response to the client.
 *
 * Sets Content-Type to application/json and adds CORS headers.
 * The response body is copied, so the caller can free json_body after.
 *
 * @param connection The MHD connection handle
 * @param status_code HTTP status code (200, 400, 404, 500, etc.)
 * @param json_body JSON string to send as response body
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result send_json_response(
    struct MHD_Connection *connection,
    int status_code,
    const char *json_body
);

/**
 * @brief Sends a JSON error response to the client.
 *
 * Convenience wrapper that formats error message as:
 * {"success": false, "error": "<message>"}
 *
 * @param connection The MHD connection handle
 * @param status_code HTTP status code (400, 404, 500, etc.)
 * @param error_message Error description to include in response
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result send_error_response(
    struct MHD_Connection *connection,
    int status_code,
    const char *error_message
);

#endif
