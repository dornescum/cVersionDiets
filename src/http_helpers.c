/**
 * @file http_helpers.c
 * @brief HTTP response helper implementations.
 */

#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include "http_helpers.h"

enum MHD_Result send_json_response(
    struct MHD_Connection *connection,
    int status_code,
    const char *json_body)
{
    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(
        strlen(json_body),
        (void *)json_body,
        MHD_RESPMEM_MUST_COPY
    );

    if (response == NULL) {
        return MHD_NO;
    }

    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods",
                            "GET, POST, PUT, DELETE, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers",
                            "Content-Type");

    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

enum MHD_Result send_error_response(
    struct MHD_Connection *connection,
    int status_code,
    const char *error_message)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "{\"success\": false, \"error\": \"%s\"}", error_message);
    return send_json_response(connection, status_code, buffer);
}
