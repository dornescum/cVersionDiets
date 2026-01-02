/**
 * @file routes.h
 * @brief HTTP route handlers for the Diet API.
 *
 * Each handler function processes a specific API endpoint,
 * queries the database, and returns a JSON response.
 */

#ifndef ROUTES_H
#define ROUTES_H

#include <microhttpd.h>

/**
 * @brief Handles GET /health endpoint.
 *
 * Returns server health status for monitoring/load balancers.
 * Response: {"status": "ok", "service": "diet-api-c"}
 *
 * @param connection The MHD connection handle
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result handle_health(struct MHD_Connection *connection);

/**
 * @brief Handles GET /api/categories endpoint.
 *
 * Returns all food categories ordered by sort_order.
 * Response: {"success": true, "categories": [...], "count": N}
 *
 * @param connection The MHD connection handle
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result handle_list_categories(struct MHD_Connection *connection);

/**
 * @brief Handles GET /api/categories/{id} endpoint.
 *
 * Returns a single category by ID.
 * Response: {"success": true, "category": {...}}
 * Error: {"success": false, "error": "Category not found"} (404)
 *
 * @param connection The MHD connection handle
 * @param id Category ID from URL path
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result handle_get_category(struct MHD_Connection *connection, int id);

/**
 * @brief Handles GET /api/foods endpoint.
 *
 * Returns food items with optional filtering.
 * Query params: category_id, search, limit (default 100, max 1000)
 * Response: {"success": true, "foods": [...], "count": N}
 *
 * @param connection The MHD connection handle
 * @return MHD_YES on success, MHD_NO on failure
 *
 * @warning The search parameter is vulnerable to SQL injection.
 *          Use mysql_real_escape_string() before production.
 */
enum MHD_Result handle_list_foods(struct MHD_Connection *connection);

/**
 * @brief Handles GET /api/foods/{id} endpoint.
 *
 * Returns a single food item by ID.
 * Response: {"success": true, "food": {...}}
 * Error: {"success": false, "error": "Food not found"} (404)
 *
 * @param connection The MHD connection handle
 * @param id Food item ID from URL path
 * @return MHD_YES on success, MHD_NO on failure
 */
enum MHD_Result handle_get_food(struct MHD_Connection *connection, int id);

#endif
