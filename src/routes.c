/**
 * @file routes.c
 * @brief HTTP route handler implementations.
 *
 * Contains all API endpoint handlers that query the database
 * and return JSON responses.
 */

#include <microhttpd.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "routes.h"
#include "http_helpers.h"
#include "db.h"

enum MHD_Result handle_health(struct MHD_Connection *connection) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "status", "ok");
    cJSON_AddStringToObject(root, "service", "diet-api-c");

    char *json_str = cJSON_PrintUnformatted(root);
    enum MHD_Result ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_list_categories(struct MHD_Connection *connection) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *categories, *item;
    char *json_str;
    enum MHD_Result ret;

    result = db_query(
        "SELECT id, name, icon, color, sort_order "
        "FROM food_categories ORDER BY sort_order"
    );

    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    categories = cJSON_AddArrayToObject(root, "categories");

    while ((row = mysql_fetch_row(result)) != NULL) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", atoi(row[0]));
        cJSON_AddStringToObject(item, "name", row[1] ? row[1] : "");
        cJSON_AddStringToObject(item, "icon", row[2] ? row[2] : "");
        cJSON_AddStringToObject(item, "color", row[3] ? row[3] : "");
        cJSON_AddNumberToObject(item, "sort_order", row[4] ? atoi(row[4]) : 0);
        cJSON_AddItemToArray(categories, item);
    }

    cJSON_AddNumberToObject(root, "count", cJSON_GetArraySize(categories));

    mysql_free_result(result);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_get_category(struct MHD_Connection *connection, int id) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *category;
    char *json_str;
    enum MHD_Result ret;
    char query[256];

    snprintf(query, sizeof(query),
        "SELECT id, name, icon, color, sort_order "
        "FROM food_categories WHERE id = %d", id);

    result = db_query(query);

    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        return send_error_response(connection, 404, "Category not found");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);

    category = cJSON_AddObjectToObject(root, "category");
    cJSON_AddNumberToObject(category, "id", atoi(row[0]));
    cJSON_AddStringToObject(category, "name", row[1] ? row[1] : "");
    cJSON_AddStringToObject(category, "icon", row[2] ? row[2] : "");
    cJSON_AddStringToObject(category, "color", row[3] ? row[3] : "");
    cJSON_AddNumberToObject(category, "sort_order", row[4] ? atoi(row[4]) : 0);

    mysql_free_result(result);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_list_foods(struct MHD_Connection *connection) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *foods, *item;
    char *json_str;
    enum MHD_Result ret;

    /* Get query parameters */
    const char *category_id_str = MHD_lookup_connection_value(
        connection, MHD_GET_ARGUMENT_KIND, "category_id");
    const char *search = MHD_lookup_connection_value(
        connection, MHD_GET_ARGUMENT_KIND, "search");
    const char *limit_str = MHD_lookup_connection_value(
        connection, MHD_GET_ARGUMENT_KIND, "limit");

    char query[1024];
    char where_clause[512] = "";
    int has_where = 0;

    /* Build WHERE clause for category filter */
    if (category_id_str != NULL) {
        snprintf(where_clause, sizeof(where_clause),
            " WHERE category_id = %d", atoi(category_id_str));
        has_where = 1;
    }

    /*
     * @warning SQL injection vulnerability!
     * TODO: Use mysql_real_escape_string() for search parameter
     */
    if (search != NULL && strlen(search) > 0) {
        if (has_where) {
            snprintf(where_clause + strlen(where_clause),
                sizeof(where_clause) - strlen(where_clause),
                " AND name LIKE '%%%s%%'", search);
        } else {
            snprintf(where_clause, sizeof(where_clause),
                " WHERE name LIKE '%%%s%%'", search);
        }
    }

    /* Parse and validate limit parameter */
    int limit = 100;
    if (limit_str != NULL) {
        limit = atoi(limit_str);
        if (limit <= 0 || limit > 1000) limit = 100;
    }

    snprintf(query, sizeof(query),
        "SELECT id, name, category_id, calories_per_100g, protein_per_100g, "
        "carbs_per_100g, fat_per_100g FROM food_items%s ORDER BY name LIMIT %d",
        where_clause, limit);

    result = db_query(query);

    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    foods = cJSON_AddArrayToObject(root, "foods");

    while ((row = mysql_fetch_row(result)) != NULL) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", atoi(row[0]));
        cJSON_AddStringToObject(item, "name", row[1] ? row[1] : "");
        cJSON_AddNumberToObject(item, "category_id", row[2] ? atoi(row[2]) : 0);
        cJSON_AddNumberToObject(item, "calories", row[3] ? atof(row[3]) : 0);
        cJSON_AddNumberToObject(item, "protein", row[4] ? atof(row[4]) : 0);
        cJSON_AddNumberToObject(item, "carbs", row[5] ? atof(row[5]) : 0);
        cJSON_AddNumberToObject(item, "fat", row[6] ? atof(row[6]) : 0);
        cJSON_AddItemToArray(foods, item);
    }

    cJSON_AddNumberToObject(root, "count", cJSON_GetArraySize(foods));

    mysql_free_result(result);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_get_food(struct MHD_Connection *connection, int id) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *food;
    char *json_str;
    enum MHD_Result ret;
    char query[256];

    snprintf(query, sizeof(query),
        "SELECT id, name, category_id, calories_per_100g, protein_per_100g, "
        "carbs_per_100g, fat_per_100g FROM food_items WHERE id = %d", id);

    result = db_query(query);

    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        return send_error_response(connection, 404, "Food not found");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);

    food = cJSON_AddObjectToObject(root, "food");
    cJSON_AddNumberToObject(food, "id", atoi(row[0]));
    cJSON_AddStringToObject(food, "name", row[1] ? row[1] : "");
    cJSON_AddNumberToObject(food, "category_id", row[2] ? atoi(row[2]) : 0);
    cJSON_AddNumberToObject(food, "calories", row[3] ? atof(row[3]) : 0);
    cJSON_AddNumberToObject(food, "protein", row[4] ? atof(row[4]) : 0);
    cJSON_AddNumberToObject(food, "carbs", row[5] ? atof(row[5]) : 0);
    cJSON_AddNumberToObject(food, "fat", row[6] ? atof(row[6]) : 0);

    mysql_free_result(result);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_get_template_full(struct MHD_Connection *connection, int id) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    cJSON *root, *template_obj, *days_arr, *day_obj, *meals_arr, *meal_obj, *items_arr, *item_obj;
    char *json_str;
    enum MHD_Result ret;
    char query[512];

    /* Get template */
    snprintf(query, sizeof(query),
        "SELECT id, code, name, description, segment, type, duration_days, calories_target "
        "FROM diet_templates WHERE id = %d", id);

    result = db_query(query);
    if (result == NULL) {
        return send_error_response(connection, 500, "Database error");
    }

    row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        return send_error_response(connection, 404, "Template not found");
    }

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);

    template_obj = cJSON_AddObjectToObject(root, "template");
    cJSON_AddNumberToObject(template_obj, "id", atoi(row[0]));
    cJSON_AddStringToObject(template_obj, "code", row[1] ? row[1] : "");
    cJSON_AddStringToObject(template_obj, "name", row[2] ? row[2] : "");
    cJSON_AddStringToObject(template_obj, "description", row[3] ? row[3] : "");
    cJSON_AddStringToObject(template_obj, "segment", row[4] ? row[4] : "");
    cJSON_AddStringToObject(template_obj, "type", row[5] ? row[5] : "");
    cJSON_AddNumberToObject(template_obj, "duration_days", row[6] ? atoi(row[6]) : 0);
    cJSON_AddNumberToObject(template_obj, "calories_target", row[7] ? atoi(row[7]) : 0);

    mysql_free_result(result);

    /* Get days */
    days_arr = cJSON_AddArrayToObject(template_obj, "days");

    snprintf(query, sizeof(query),
        "SELECT id, day_number, day_name FROM diet_days WHERE template_id = %d ORDER BY day_number", id);

    result = db_query(query);
    if (result == NULL) {
        cJSON_Delete(root);
        return send_error_response(connection, 500, "Database error");
    }

    /* Store day IDs for meal queries */
    int day_ids[100];
    int day_count = 0;

    while ((row = mysql_fetch_row(result)) != NULL && day_count < 100) {
        day_obj = cJSON_CreateObject();
        day_ids[day_count] = atoi(row[0]);
        cJSON_AddNumberToObject(day_obj, "id", day_ids[day_count]);
        cJSON_AddNumberToObject(day_obj, "day_number", row[1] ? atoi(row[1]) : 0);
        cJSON_AddStringToObject(day_obj, "day_name", row[2] ? row[2] : "");
        cJSON_AddArrayToObject(day_obj, "meals");
        cJSON_AddItemToArray(days_arr, day_obj);
        day_count++;
    }
    mysql_free_result(result);

    /* Get meals for each day */
    for (int i = 0; i < day_count; i++) {
        day_obj = cJSON_GetArrayItem(days_arr, i);
        meals_arr = cJSON_GetObjectItem(day_obj, "meals");

        snprintf(query, sizeof(query),
            "SELECT id, meal_type, meal_order, time_suggestion "
            "FROM diet_meals WHERE day_id = %d ORDER BY meal_order", day_ids[i]);

        result = db_query(query);
        if (result == NULL) continue;

        int meal_ids[50];
        int meal_count = 0;

        while ((row = mysql_fetch_row(result)) != NULL && meal_count < 50) {
            meal_obj = cJSON_CreateObject();
            meal_ids[meal_count] = atoi(row[0]);
            cJSON_AddNumberToObject(meal_obj, "id", meal_ids[meal_count]);
            cJSON_AddStringToObject(meal_obj, "meal_type", row[1] ? row[1] : "");
            cJSON_AddNumberToObject(meal_obj, "meal_order", row[2] ? atoi(row[2]) : 0);
            cJSON_AddStringToObject(meal_obj, "time_suggestion", row[3] ? row[3] : "");
            cJSON_AddArrayToObject(meal_obj, "items");
            cJSON_AddItemToArray(meals_arr, meal_obj);
            meal_count++;
        }
        mysql_free_result(result);

        /* Get items for each meal */
        for (int j = 0; j < meal_count; j++) {
            meal_obj = cJSON_GetArrayItem(meals_arr, j);
            items_arr = cJSON_GetObjectItem(meal_obj, "items");

            snprintf(query, sizeof(query),
                "SELECT mi.id, mi.food_item_id, f.name, mi.portion_grams_min, mi.portion_grams_max "
                "FROM diet_meal_items mi "
                "JOIN food_items f ON mi.food_item_id = f.id "
                "WHERE mi.meal_id = %d ORDER BY mi.sort_order", meal_ids[j]);

            result = db_query(query);
            if (result == NULL) continue;

            while ((row = mysql_fetch_row(result)) != NULL) {
                item_obj = cJSON_CreateObject();
                cJSON_AddNumberToObject(item_obj, "id", atoi(row[0]));
                cJSON_AddNumberToObject(item_obj, "food_item_id", row[1] ? atoi(row[1]) : 0);
                cJSON_AddStringToObject(item_obj, "food_name", row[2] ? row[2] : "");
                cJSON_AddNumberToObject(item_obj, "portion_grams_min", row[3] ? atoi(row[3]) : 0);
                cJSON_AddNumberToObject(item_obj, "portion_grams_max", row[4] ? atoi(row[4]) : 0);
                cJSON_AddItemToArray(items_arr, item_obj);
            }
            mysql_free_result(result);
        }
    }

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 200, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}

enum MHD_Result handle_bulk_insert(struct MHD_Connection *connection,
                                   const char *post_data, size_t post_data_size) {
    (void)post_data_size;
    cJSON *root, *json_input, *items_arr, *item;
    char *json_str;
    enum MHD_Result ret;
    char query[1024];
    int inserted = 0;

    if (post_data == NULL) {
        return send_error_response(connection, 400, "Missing request body");
    }

    json_input = cJSON_Parse(post_data);
    if (json_input == NULL) {
        return send_error_response(connection, 400, "Invalid JSON");
    }

    cJSON *meal_id_json = cJSON_GetObjectItem(json_input, "meal_id");
    items_arr = cJSON_GetObjectItem(json_input, "items");

    if (!cJSON_IsNumber(meal_id_json) || !cJSON_IsArray(items_arr)) {
        cJSON_Delete(json_input);
        return send_error_response(connection, 400, "Invalid request format");
    }

    int meal_id = meal_id_json->valueint;
    int items_count = cJSON_GetArraySize(items_arr);

    /* Insert each item */
    for (int i = 0; i < items_count; i++) {
        item = cJSON_GetArrayItem(items_arr, i);

        cJSON *food_id = cJSON_GetObjectItem(item, "food_item_id");
        cJSON *portion_min = cJSON_GetObjectItem(item, "portion_grams_min");
        cJSON *portion_max = cJSON_GetObjectItem(item, "portion_grams_max");
        cJSON *sort_order = cJSON_GetObjectItem(item, "sort_order");

        if (!cJSON_IsNumber(food_id) || !cJSON_IsNumber(portion_min) || !cJSON_IsNumber(portion_max)) {
            continue;
        }

        snprintf(query, sizeof(query),
            "INSERT INTO diet_meal_items (meal_id, food_item_id, portion_grams_min, portion_grams_max, sort_order) "
            "VALUES (%d, %d, %d, %d, %d)",
            meal_id,
            food_id->valueint,
            portion_min->valueint,
            portion_max->valueint,
            cJSON_IsNumber(sort_order) ? sort_order->valueint : i);

        if (db_execute(query) >= 0) {
            inserted++;
        }
    }

    cJSON_Delete(json_input);

    root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    cJSON_AddNumberToObject(root, "inserted_count", inserted);

    json_str = cJSON_PrintUnformatted(root);
    ret = send_json_response(connection, 201, json_str);

    free(json_str);
    cJSON_Delete(root);

    return ret;
}
