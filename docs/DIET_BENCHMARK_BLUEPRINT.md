# Diet API Benchmark Blueprint

A multi-language, multi-database performance comparison project for portfolio demonstration.

**Goal:** Implement the same Diet API in multiple languages, benchmark against multiple databases, and present objective performance comparisons.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [OpenAPI Specification](#2-openapi-specification)
3. [Project Structure](#3-project-structure)
4. [Database Schema](#4-database-schema)
5. [Benchmark Scripts](#5-benchmark-scripts)
6. [Docker Configuration](#6-docker-configuration)
7. [VPS Deployment Strategy](#7-vps-deployment-strategy)
8. [Results Template](#8-results-template)

---

## 1. Project Overview

### Implementations

| Language | Framework | Priority |
|----------|-----------|----------|
| Node.js | Express | Baseline (done) |
| Node.js | Bun + Hono | Low effort comparison |
| Python | FastAPI | Popular async framework |
| Go | Fiber/Gin | High performance, trending |
| C | libmicrohttpd + libmysqlclient | Systems showcase |
| C++ | Drogon/Crow | Modern C++ web |

### Databases

| Database | Priority | Notes |
|----------|----------|-------|
| MySQL 8 | Primary | Current implementation |
| PostgreSQL 15 | Secondary | Popular alternative |
| SQLite 3 | Tertiary | Embedded, fast for reads |

### What We're Benchmarking

1. **Simple CRUD** - Food categories (small table, ~10 rows)
2. **Paginated List** - Food items with filters (medium, ~1000 rows)
3. **Complex Join** - Full diet template (template → days → meals → items)
4. **Bulk Insert** - Create 100 meal items in one request

---

## 2. OpenAPI Specification

```yaml
openapi: 3.0.3
info:
  title: Diet Benchmark API
  description: |
    Standardized API contract for diet management system.
    All implementations must conform to this specification.
  version: 1.0.0
  contact:
    name: Mihai Dornescu
    url: https://github.com/yourusername

servers:
  - url: http://localhost:{port}
    variables:
      port:
        default: "3000"
        enum: ["3000", "3001", "3002", "8000", "8080"]

tags:
  - name: Health
    description: Health check endpoints
  - name: Categories
    description: Food category management
  - name: Foods
    description: Food item management
  - name: Templates
    description: Diet template management
  - name: Benchmark
    description: Bulk operations for benchmarking

paths:
  # ============================================
  # HEALTH CHECK
  # ============================================
  /health:
    get:
      tags: [Health]
      summary: Health check
      operationId: healthCheck
      responses:
        "200":
          description: Service is healthy
          content:
            application/json:
              schema:
                type: object
                properties:
                  status:
                    type: string
                    example: ok
                  timestamp:
                    type: string
                    format: date-time
                  database:
                    type: string
                    example: connected

  # ============================================
  # FOOD CATEGORIES
  # ============================================
  /api/categories:
    get:
      tags: [Categories]
      summary: List all food categories
      operationId: listCategories
      responses:
        "200":
          description: List of categories
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/CategoryListResponse"

    post:
      tags: [Categories]
      summary: Create a food category
      operationId: createCategory
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/CategoryCreate"
      responses:
        "201":
          description: Category created
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/CategoryResponse"
        "400":
          $ref: "#/components/responses/ValidationError"

  /api/categories/{id}:
    parameters:
      - $ref: "#/components/parameters/CategoryId"

    get:
      tags: [Categories]
      summary: Get a category by ID
      operationId: getCategory
      responses:
        "200":
          description: Category found
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/CategoryResponse"
        "404":
          $ref: "#/components/responses/NotFound"

    put:
      tags: [Categories]
      summary: Update a category
      operationId: updateCategory
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/CategoryUpdate"
      responses:
        "200":
          description: Category updated
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/CategoryResponse"
        "404":
          $ref: "#/components/responses/NotFound"

    delete:
      tags: [Categories]
      summary: Delete a category
      operationId: deleteCategory
      responses:
        "204":
          description: Category deleted
        "404":
          $ref: "#/components/responses/NotFound"
        "409":
          description: Category has food items (cannot delete)
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/Error"

  # ============================================
  # FOOD ITEMS
  # ============================================
  /api/foods:
    get:
      tags: [Foods]
      summary: List food items with pagination and filters
      operationId: listFoods
      parameters:
        - name: page
          in: query
          schema:
            type: integer
            default: 1
            minimum: 1
        - name: limit
          in: query
          schema:
            type: integer
            default: 20
            minimum: 1
            maximum: 100
        - name: category_id
          in: query
          schema:
            type: integer
        - name: search
          in: query
          description: Search by name
          schema:
            type: string
            maxLength: 100
        - name: is_snack_suitable
          in: query
          schema:
            type: boolean
      responses:
        "200":
          description: Paginated list of food items
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/FoodListResponse"

    post:
      tags: [Foods]
      summary: Create a food item
      operationId: createFood
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/FoodCreate"
      responses:
        "201":
          description: Food item created
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/FoodResponse"
        "400":
          $ref: "#/components/responses/ValidationError"

  /api/foods/{id}:
    parameters:
      - $ref: "#/components/parameters/FoodId"

    get:
      tags: [Foods]
      summary: Get a food item by ID
      operationId: getFood
      responses:
        "200":
          description: Food item found
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/FoodResponse"
        "404":
          $ref: "#/components/responses/NotFound"

    put:
      tags: [Foods]
      summary: Update a food item
      operationId: updateFood
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/FoodUpdate"
      responses:
        "200":
          description: Food item updated
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/FoodResponse"
        "404":
          $ref: "#/components/responses/NotFound"

    delete:
      tags: [Foods]
      summary: Delete a food item
      operationId: deleteFood
      responses:
        "204":
          description: Food item deleted
        "404":
          $ref: "#/components/responses/NotFound"
        "409":
          description: Food item is used in diet meals

  # ============================================
  # DIET TEMPLATES
  # ============================================
  /api/templates:
    get:
      tags: [Templates]
      summary: List diet templates
      operationId: listTemplates
      parameters:
        - name: page
          in: query
          schema:
            type: integer
            default: 1
        - name: limit
          in: query
          schema:
            type: integer
            default: 20
        - name: segment
          in: query
          description: Weight segment filter (A/B/C/D)
          schema:
            type: string
            enum: [A, B, C, D]
        - name: type
          in: query
          description: Diet type filter
          schema:
            type: string
            enum: [SCR, LGI, KTP]
      responses:
        "200":
          description: List of templates
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/TemplateListResponse"

    post:
      tags: [Templates]
      summary: Create a diet template
      operationId: createTemplate
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/TemplateCreate"
      responses:
        "201":
          description: Template created
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/TemplateResponse"

  /api/templates/{id}:
    parameters:
      - $ref: "#/components/parameters/TemplateId"

    get:
      tags: [Templates]
      summary: Get template basic info
      operationId: getTemplate
      responses:
        "200":
          description: Template found
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/TemplateResponse"
        "404":
          $ref: "#/components/responses/NotFound"

    put:
      tags: [Templates]
      summary: Update template
      operationId: updateTemplate
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/TemplateUpdate"
      responses:
        "200":
          description: Template updated
        "404":
          $ref: "#/components/responses/NotFound"

    delete:
      tags: [Templates]
      summary: Delete template
      operationId: deleteTemplate
      responses:
        "204":
          description: Template deleted
        "404":
          $ref: "#/components/responses/NotFound"

  /api/templates/{id}/full:
    parameters:
      - $ref: "#/components/parameters/TemplateId"

    get:
      tags: [Templates]
      summary: Get full template with all days, meals, and food items
      description: |
        This is the "expensive" query for benchmarking.
        Returns complete nested structure:
        template → days[] → meals[] → items[] (with food details)
      operationId: getTemplateFull
      responses:
        "200":
          description: Full template with nested data
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/TemplateFullResponse"
        "404":
          $ref: "#/components/responses/NotFound"

  # ============================================
  # TEMPLATE DAYS
  # ============================================
  /api/templates/{id}/days:
    parameters:
      - $ref: "#/components/parameters/TemplateId"

    post:
      tags: [Templates]
      summary: Add a day to template
      operationId: createDay
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/DayCreate"
      responses:
        "201":
          description: Day created
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/DayResponse"

  /api/templates/{templateId}/days/{dayId}:
    parameters:
      - $ref: "#/components/parameters/TemplateId"
      - name: dayId
        in: path
        required: true
        schema:
          type: integer

    delete:
      tags: [Templates]
      summary: Delete a day from template
      operationId: deleteDay
      responses:
        "204":
          description: Day deleted

  # ============================================
  # MEALS
  # ============================================
  /api/days/{dayId}/meals:
    parameters:
      - name: dayId
        in: path
        required: true
        schema:
          type: integer

    post:
      tags: [Templates]
      summary: Add a meal to day
      operationId: createMeal
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/MealCreate"
      responses:
        "201":
          description: Meal created

  /api/meals/{mealId}:
    parameters:
      - name: mealId
        in: path
        required: true
        schema:
          type: integer

    delete:
      tags: [Templates]
      summary: Delete a meal
      operationId: deleteMeal
      responses:
        "204":
          description: Meal deleted

  # ============================================
  # MEAL ITEMS
  # ============================================
  /api/meals/{mealId}/items:
    parameters:
      - name: mealId
        in: path
        required: true
        schema:
          type: integer

    post:
      tags: [Templates]
      summary: Add food item to meal
      operationId: createMealItem
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/MealItemCreate"
      responses:
        "201":
          description: Item added to meal

  /api/meal-items/{itemId}:
    parameters:
      - name: itemId
        in: path
        required: true
        schema:
          type: integer

    delete:
      tags: [Templates]
      summary: Remove item from meal
      operationId: deleteMealItem
      responses:
        "204":
          description: Item removed

  # ============================================
  # BENCHMARK ENDPOINTS
  # ============================================
  /api/benchmark/bulk-insert:
    post:
      tags: [Benchmark]
      summary: Bulk insert meal items (for benchmarking)
      description: Insert multiple meal items in a single transaction
      operationId: bulkInsertMealItems
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              required: [meal_id, items]
              properties:
                meal_id:
                  type: integer
                items:
                  type: array
                  minItems: 1
                  maxItems: 1000
                  items:
                    $ref: "#/components/schemas/MealItemCreate"
      responses:
        "201":
          description: Items inserted
          content:
            application/json:
              schema:
                type: object
                properties:
                  success:
                    type: boolean
                  inserted_count:
                    type: integer

  /api/benchmark/complex-query:
    get:
      tags: [Benchmark]
      summary: Execute complex aggregation query
      description: |
        Returns nutritional totals per day across all templates.
        Tests JOIN performance across multiple tables.
      operationId: complexQuery
      responses:
        "200":
          description: Aggregated data
          content:
            application/json:
              schema:
                type: object
                properties:
                  data:
                    type: array
                    items:
                      type: object
                      properties:
                        template_id:
                          type: integer
                        template_name:
                          type: string
                        day_number:
                          type: integer
                        total_calories:
                          type: number
                        total_protein:
                          type: number
                        total_carbs:
                          type: number
                        total_fat:
                          type: number
                        item_count:
                          type: integer

# ============================================
# COMPONENTS
# ============================================
components:
  parameters:
    CategoryId:
      name: id
      in: path
      required: true
      schema:
        type: integer
        minimum: 1

    FoodId:
      name: id
      in: path
      required: true
      schema:
        type: integer
        minimum: 1

    TemplateId:
      name: id
      in: path
      required: true
      schema:
        type: integer
        minimum: 1

  responses:
    NotFound:
      description: Resource not found
      content:
        application/json:
          schema:
            $ref: "#/components/schemas/Error"

    ValidationError:
      description: Validation error
      content:
        application/json:
          schema:
            $ref: "#/components/schemas/ValidationErrors"

  schemas:
    # ----------------------------------------
    # Common
    # ----------------------------------------
    Error:
      type: object
      properties:
        success:
          type: boolean
          example: false
        error:
          type: string
        message:
          type: string

    ValidationErrors:
      type: object
      properties:
        success:
          type: boolean
          example: false
        errors:
          type: array
          items:
            type: object
            properties:
              field:
                type: string
              message:
                type: string

    Pagination:
      type: object
      properties:
        page:
          type: integer
        limit:
          type: integer
        total:
          type: integer
        total_pages:
          type: integer

    # ----------------------------------------
    # Category Schemas
    # ----------------------------------------
    Category:
      type: object
      properties:
        id:
          type: integer
        name:
          type: string
        icon:
          type: string
          nullable: true
        color:
          type: string
          nullable: true
        sort_order:
          type: integer

    CategoryCreate:
      type: object
      required: [name]
      properties:
        name:
          type: string
          minLength: 1
          maxLength: 50
        icon:
          type: string
          maxLength: 50
        color:
          type: string
          maxLength: 20
        sort_order:
          type: integer
          default: 0

    CategoryUpdate:
      type: object
      properties:
        name:
          type: string
          minLength: 1
          maxLength: 50
        icon:
          type: string
          maxLength: 50
        color:
          type: string
          maxLength: 20
        sort_order:
          type: integer

    CategoryResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          $ref: "#/components/schemas/Category"

    CategoryListResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          type: array
          items:
            $ref: "#/components/schemas/Category"

    # ----------------------------------------
    # Food Schemas
    # ----------------------------------------
    Food:
      type: object
      properties:
        id:
          type: integer
        category_id:
          type: integer
        category_name:
          type: string
        name:
          type: string
        description:
          type: string
          nullable: true
        default_portion_grams:
          type: integer
        calories_per_100g:
          type: number
          nullable: true
        protein_per_100g:
          type: number
          nullable: true
        carbs_per_100g:
          type: number
          nullable: true
        fat_per_100g:
          type: number
          nullable: true
        fiber_per_100g:
          type: number
          nullable: true
        is_snack_suitable:
          type: boolean
        status:
          type: integer
        created_at:
          type: string
          format: date-time

    FoodCreate:
      type: object
      required: [category_id, name]
      properties:
        category_id:
          type: integer
        name:
          type: string
          minLength: 1
          maxLength: 100
        description:
          type: string
          maxLength: 255
        default_portion_grams:
          type: integer
          default: 100
        calories_per_100g:
          type: number
        protein_per_100g:
          type: number
        carbs_per_100g:
          type: number
        fat_per_100g:
          type: number
        fiber_per_100g:
          type: number
        is_snack_suitable:
          type: boolean
          default: false

    FoodUpdate:
      type: object
      properties:
        category_id:
          type: integer
        name:
          type: string
          maxLength: 100
        description:
          type: string
          maxLength: 255
        default_portion_grams:
          type: integer
        calories_per_100g:
          type: number
        protein_per_100g:
          type: number
        carbs_per_100g:
          type: number
        fat_per_100g:
          type: number
        fiber_per_100g:
          type: number
        is_snack_suitable:
          type: boolean

    FoodResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          $ref: "#/components/schemas/Food"

    FoodListResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          type: array
          items:
            $ref: "#/components/schemas/Food"
        pagination:
          $ref: "#/components/schemas/Pagination"

    # ----------------------------------------
    # Template Schemas
    # ----------------------------------------
    Template:
      type: object
      properties:
        id:
          type: integer
        code:
          type: string
        name:
          type: string
        description:
          type: string
          nullable: true
        segment:
          type: string
          enum: [A, B, C, D]
        type:
          type: string
        duration_days:
          type: integer
        calories_target:
          type: integer
          nullable: true
        status:
          type: integer
        created_at:
          type: string
          format: date-time

    TemplateCreate:
      type: object
      required: [code, name, segment, type]
      properties:
        code:
          type: string
          minLength: 1
          maxLength: 50
        name:
          type: string
          minLength: 1
          maxLength: 100
        description:
          type: string
        segment:
          type: string
          enum: [A, B, C, D]
        type:
          type: string
          maxLength: 50
        duration_days:
          type: integer
          default: 30
        calories_target:
          type: integer

    TemplateUpdate:
      type: object
      properties:
        name:
          type: string
          maxLength: 100
        description:
          type: string
        segment:
          type: string
          enum: [A, B, C, D]
        type:
          type: string
        duration_days:
          type: integer
        calories_target:
          type: integer

    TemplateResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          $ref: "#/components/schemas/Template"

    TemplateListResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          type: array
          items:
            $ref: "#/components/schemas/Template"
        pagination:
          $ref: "#/components/schemas/Pagination"

    # Full template with nested data
    TemplateFullResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          type: object
          properties:
            id:
              type: integer
            code:
              type: string
            name:
              type: string
            description:
              type: string
            segment:
              type: string
            type:
              type: string
            duration_days:
              type: integer
            calories_target:
              type: integer
            days:
              type: array
              items:
                type: object
                properties:
                  id:
                    type: integer
                  day_number:
                    type: integer
                  day_name:
                    type: string
                  meals:
                    type: array
                    items:
                      type: object
                      properties:
                        id:
                          type: integer
                        meal_type:
                          type: string
                          enum: [breakfast, lunch, dinner, snack]
                        meal_order:
                          type: integer
                        time_suggestion:
                          type: string
                        items:
                          type: array
                          items:
                            type: object
                            properties:
                              id:
                                type: integer
                              food_item_id:
                                type: integer
                              food_name:
                                type: string
                              category_name:
                                type: string
                              portion_grams_min:
                                type: integer
                              portion_grams_max:
                                type: integer
                              calories_per_100g:
                                type: number
                              protein_per_100g:
                                type: number
                              carbs_per_100g:
                                type: number
                              fat_per_100g:
                                type: number

    # ----------------------------------------
    # Day/Meal Schemas
    # ----------------------------------------
    DayCreate:
      type: object
      required: [day_number]
      properties:
        day_number:
          type: integer
          minimum: 1
        day_name:
          type: string
          maxLength: 20
        notes:
          type: string

    DayResponse:
      type: object
      properties:
        success:
          type: boolean
        data:
          type: object
          properties:
            id:
              type: integer
            template_id:
              type: integer
            day_number:
              type: integer
            day_name:
              type: string

    MealCreate:
      type: object
      required: [meal_type]
      properties:
        meal_type:
          type: string
          enum: [breakfast, lunch, dinner, snack]
        meal_order:
          type: integer
          default: 0
        time_suggestion:
          type: string
          maxLength: 10
        notes:
          type: string

    MealItemCreate:
      type: object
      required: [food_item_id, portion_grams_min, portion_grams_max]
      properties:
        food_item_id:
          type: integer
        portion_grams_min:
          type: integer
          minimum: 1
        portion_grams_max:
          type: integer
          minimum: 1
        portion_description:
          type: string
          maxLength: 100
        preparation_notes:
          type: string
          maxLength: 255
        is_optional:
          type: boolean
          default: false
        sort_order:
          type: integer
          default: 0
```

---

## 3. Project Structure

```
diet-benchmark/
├── README.md                         # Project overview, results, how to run
├── LICENSE                           # MIT or similar
├── .gitignore
│
├── api-spec.yaml                     # OpenAPI specification (above)
│
├── docker/
│   ├── docker-compose.yml            # Orchestrates everything
│   ├── docker-compose.mysql.yml      # MySQL-specific
│   ├── docker-compose.postgres.yml   # PostgreSQL-specific
│   └── docker-compose.sqlite.yml     # SQLite-specific
│
├── databases/
│   ├── mysql/
│   │   ├── init.sql                  # Schema for MySQL
│   │   └── seed.sql                  # Seed data
│   ├── postgres/
│   │   ├── init.sql                  # Schema for PostgreSQL
│   │   └── seed.sql                  # Seed data
│   └── sqlite/
│       ├── schema.sql                # Schema for SQLite
│       └── seed.sql                  # Seed data
│
├── implementations/
│   ├── nodejs-express/
│   │   ├── Dockerfile
│   │   ├── package.json
│   │   ├── src/
│   │   │   ├── index.js
│   │   │   ├── routes/
│   │   │   ├── controllers/
│   │   │   └── models/
│   │   └── .env.example
│   │
│   ├── nodejs-bun/
│   │   ├── Dockerfile
│   │   ├── package.json
│   │   └── src/
│   │
│   ├── python-fastapi/
│   │   ├── Dockerfile
│   │   ├── requirements.txt
│   │   ├── app/
│   │   │   ├── main.py
│   │   │   ├── routers/
│   │   │   ├── models/
│   │   │   └── database.py
│   │   └── .env.example
│   │
│   ├── go-fiber/
│   │   ├── Dockerfile
│   │   ├── go.mod
│   │   ├── main.go
│   │   ├── handlers/
│   │   ├── models/
│   │   └── database/
│   │
│   ├── c-microhttpd/
│   │   ├── Dockerfile
│   │   ├── Makefile
│   │   ├── src/
│   │   │   ├── main.c
│   │   │   ├── routes.c
│   │   │   ├── db.c
│   │   │   └── json.c
│   │   └── README.md
│   │
│   └── cpp-drogon/
│       ├── Dockerfile
│       ├── CMakeLists.txt
│       ├── src/
│       └── README.md
│
├── benchmarks/
│   ├── run-benchmark.sh              # Main benchmark runner
│   ├── k6/
│   │   ├── categories.js             # Simple CRUD test
│   │   ├── foods-list.js             # Paginated list test
│   │   ├── template-full.js          # Complex join test
│   │   ├── bulk-insert.js            # Bulk insert test
│   │   └── full-suite.js             # All tests combined
│   ├── wrk/
│   │   └── scripts/                  # Lua scripts for wrk
│   └── results/
│       └── .gitkeep
│
├── scripts/
│   ├── setup-vps.sh                  # VPS initial setup
│   ├── deploy.sh                     # Deploy all implementations
│   ├── generate-seed-data.js         # Generate consistent seed data
│   └── export-results.js             # Convert results to charts
│
└── docs/
    ├── IMPLEMENTATION_GUIDE.md       # How to add a new implementation
    ├── BENCHMARK_METHODOLOGY.md      # How benchmarks are run
    └── RESULTS.md                    # Detailed results analysis
```

---

## 4. Database Schema

### Shared Schema (adapt syntax per database)

```sql
-- ============================================
-- FOOD CATEGORIES
-- ============================================
CREATE TABLE food_categories (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL,
    icon VARCHAR(50),
    color VARCHAR(20),
    sort_order INTEGER DEFAULT 0
);

-- ============================================
-- FOOD ITEMS
-- ============================================
CREATE TABLE food_items (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    category_id INTEGER NOT NULL,
    name VARCHAR(100) NOT NULL,
    description VARCHAR(255),
    default_portion_grams INTEGER NOT NULL DEFAULT 100,
    calories_per_100g DECIMAL(6,2),
    protein_per_100g DECIMAL(5,2),
    carbs_per_100g DECIMAL(5,2),
    fat_per_100g DECIMAL(5,2),
    fiber_per_100g DECIMAL(5,2),
    is_snack_suitable BOOLEAN DEFAULT FALSE,
    status TINYINT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES food_categories(id)
);

CREATE INDEX idx_food_items_category ON food_items(category_id);
CREATE INDEX idx_food_items_name ON food_items(name);

-- ============================================
-- DIET TEMPLATES
-- ============================================
CREATE TABLE diet_templates (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    code VARCHAR(50) NOT NULL UNIQUE,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    segment CHAR(1) NOT NULL,
    type VARCHAR(50) NOT NULL,
    duration_days INTEGER NOT NULL DEFAULT 30,
    calories_target INTEGER,
    notes TEXT,
    status TINYINT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE INDEX idx_templates_segment ON diet_templates(segment);
CREATE INDEX idx_templates_type ON diet_templates(type);

-- ============================================
-- DIET DAYS
-- ============================================
CREATE TABLE diet_days (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    template_id INTEGER NOT NULL,
    day_number INTEGER NOT NULL,
    day_name VARCHAR(20),
    notes TEXT,
    FOREIGN KEY (template_id) REFERENCES diet_templates(id) ON DELETE CASCADE,
    UNIQUE KEY unique_day (template_id, day_number)
);

-- ============================================
-- DIET MEALS
-- ============================================
CREATE TABLE diet_meals (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    day_id INTEGER NOT NULL,
    meal_type ENUM('breakfast', 'lunch', 'dinner', 'snack') NOT NULL,
    meal_order INTEGER DEFAULT 0,
    time_suggestion VARCHAR(10),
    notes TEXT,
    FOREIGN KEY (day_id) REFERENCES diet_days(id) ON DELETE CASCADE
);

CREATE INDEX idx_meals_day ON diet_meals(day_id);

-- ============================================
-- DIET MEAL ITEMS
-- ============================================
CREATE TABLE diet_meal_items (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    meal_id INTEGER NOT NULL,
    food_item_id INTEGER NOT NULL,
    portion_grams_min INTEGER NOT NULL,
    portion_grams_max INTEGER NOT NULL,
    portion_description VARCHAR(100),
    preparation_notes VARCHAR(255),
    is_optional BOOLEAN DEFAULT FALSE,
    sort_order INTEGER DEFAULT 0,
    FOREIGN KEY (meal_id) REFERENCES diet_meals(id) ON DELETE CASCADE,
    FOREIGN KEY (food_item_id) REFERENCES food_items(id) ON DELETE RESTRICT
);

CREATE INDEX idx_meal_items_meal ON diet_meal_items(meal_id);
CREATE INDEX idx_meal_items_food ON diet_meal_items(food_item_id);
```

### PostgreSQL Adaptations

```sql
-- Use SERIAL instead of AUTO_INCREMENT
CREATE TABLE food_categories (
    id SERIAL PRIMARY KEY,
    -- ...
);

-- Use TEXT or custom ENUM for meal_type
CREATE TYPE meal_type AS ENUM ('breakfast', 'lunch', 'dinner', 'snack');

CREATE TABLE diet_meals (
    id SERIAL PRIMARY KEY,
    day_id INTEGER NOT NULL,
    meal_type meal_type NOT NULL,
    -- ...
);
```

### SQLite Adaptations

```sql
-- SQLite uses INTEGER PRIMARY KEY for auto-increment
-- No ENUM type - use CHECK constraint
CREATE TABLE diet_meals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    day_id INTEGER NOT NULL,
    meal_type TEXT NOT NULL CHECK (meal_type IN ('breakfast', 'lunch', 'dinner', 'snack')),
    -- ...
);

-- No ON UPDATE for timestamps - handle in application
```

---

## 5. Benchmark Scripts

### Main Runner Script

```bash
#!/bin/bash
# benchmarks/run-benchmark.sh

set -e

# Configuration
DURATION=${DURATION:-30s}
CONNECTIONS=${CONNECTIONS:-100}
THREADS=${THREADS:-4}
BASE_URL=${BASE_URL:-http://localhost:3000}
OUTPUT_DIR=${OUTPUT_DIR:-./results}

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== Diet API Benchmark Suite ===${NC}"
echo "Duration: $DURATION"
echo "Connections: $CONNECTIONS"
echo "Target: $BASE_URL"
echo ""

# Create output directory
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULT_DIR="$OUTPUT_DIR/$TIMESTAMP"
mkdir -p "$RESULT_DIR"

# Wait for service to be ready
echo -e "${YELLOW}Waiting for service...${NC}"
until curl -s "$BASE_URL/health" > /dev/null 2>&1; do
    sleep 1
done
echo "Service is ready!"

# ============================================
# Test 1: Simple CRUD - GET /api/categories
# ============================================
echo -e "\n${GREEN}[1/4] Testing: GET /api/categories (simple read)${NC}"
k6 run \
    --vus $CONNECTIONS \
    --duration $DURATION \
    --out json="$RESULT_DIR/categories.json" \
    -e BASE_URL=$BASE_URL \
    ./k6/categories.js

# ============================================
# Test 2: Paginated List - GET /api/foods
# ============================================
echo -e "\n${GREEN}[2/4] Testing: GET /api/foods (paginated list)${NC}"
k6 run \
    --vus $CONNECTIONS \
    --duration $DURATION \
    --out json="$RESULT_DIR/foods.json" \
    -e BASE_URL=$BASE_URL \
    ./k6/foods-list.js

# ============================================
# Test 3: Complex Join - GET /api/templates/{id}/full
# ============================================
echo -e "\n${GREEN}[3/4] Testing: GET /api/templates/1/full (complex join)${NC}"
k6 run \
    --vus $CONNECTIONS \
    --duration $DURATION \
    --out json="$RESULT_DIR/template-full.json" \
    -e BASE_URL=$BASE_URL \
    ./k6/template-full.js

# ============================================
# Test 4: Bulk Insert - POST /api/benchmark/bulk-insert
# ============================================
echo -e "\n${GREEN}[4/4] Testing: POST /api/benchmark/bulk-insert${NC}"
k6 run \
    --vus 10 \
    --duration $DURATION \
    --out json="$RESULT_DIR/bulk-insert.json" \
    -e BASE_URL=$BASE_URL \
    ./k6/bulk-insert.js

# ============================================
# Generate Summary
# ============================================
echo -e "\n${GREEN}=== Benchmark Complete ===${NC}"
echo "Results saved to: $RESULT_DIR"

# Parse results and create summary
node ../scripts/export-results.js "$RESULT_DIR"
```

### k6 Test Scripts

#### categories.js - Simple CRUD
```javascript
// benchmarks/k6/categories.js
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Rate, Trend } from 'k6/metrics';

const errorRate = new Rate('errors');
const latency = new Trend('latency');

const BASE_URL = __ENV.BASE_URL || 'http://localhost:3000';

export const options = {
    thresholds: {
        http_req_duration: ['p(95)<500'],  // 95% under 500ms
        errors: ['rate<0.01'],              // Error rate under 1%
    },
};

export default function () {
    // GET all categories
    const res = http.get(`${BASE_URL}/api/categories`);

    check(res, {
        'status is 200': (r) => r.status === 200,
        'has data': (r) => JSON.parse(r.body).success === true,
    });

    errorRate.add(res.status !== 200);
    latency.add(res.timings.duration);

    sleep(0.1);
}

export function handleSummary(data) {
    return {
        'stdout': JSON.stringify({
            test: 'categories',
            requests_per_second: data.metrics.http_reqs.values.rate,
            avg_latency_ms: data.metrics.http_req_duration.values.avg,
            p95_latency_ms: data.metrics.http_req_duration.values['p(95)'],
            p99_latency_ms: data.metrics.http_req_duration.values['p(99)'],
            error_rate: data.metrics.errors?.values?.rate || 0,
        }, null, 2),
    };
}
```

#### foods-list.js - Paginated List
```javascript
// benchmarks/k6/foods-list.js
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Rate, Trend } from 'k6/metrics';

const errorRate = new Rate('errors');
const latency = new Trend('latency');

const BASE_URL = __ENV.BASE_URL || 'http://localhost:3000';

export const options = {
    thresholds: {
        http_req_duration: ['p(95)<1000'],
        errors: ['rate<0.01'],
    },
};

export default function () {
    // Randomly select page and category
    const page = Math.floor(Math.random() * 10) + 1;
    const category = Math.floor(Math.random() * 10) + 1;

    const url = `${BASE_URL}/api/foods?page=${page}&limit=20&category_id=${category}`;
    const res = http.get(url);

    check(res, {
        'status is 200': (r) => r.status === 200,
        'has data array': (r) => {
            const body = JSON.parse(r.body);
            return body.success && Array.isArray(body.data);
        },
        'has pagination': (r) => {
            const body = JSON.parse(r.body);
            return body.pagination !== undefined;
        },
    });

    errorRate.add(res.status !== 200);
    latency.add(res.timings.duration);

    sleep(0.1);
}

export function handleSummary(data) {
    return {
        'stdout': JSON.stringify({
            test: 'foods-list',
            requests_per_second: data.metrics.http_reqs.values.rate,
            avg_latency_ms: data.metrics.http_req_duration.values.avg,
            p95_latency_ms: data.metrics.http_req_duration.values['p(95)'],
            p99_latency_ms: data.metrics.http_req_duration.values['p(99)'],
            error_rate: data.metrics.errors?.values?.rate || 0,
        }, null, 2),
    };
}
```

#### template-full.js - Complex Join
```javascript
// benchmarks/k6/template-full.js
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Rate, Trend } from 'k6/metrics';

const errorRate = new Rate('errors');
const latency = new Trend('latency');

const BASE_URL = __ENV.BASE_URL || 'http://localhost:3000';

// Template IDs to test (assumes 1-6 exist from seed data)
const TEMPLATE_IDS = [1, 2, 3, 4, 5, 6];

export const options = {
    thresholds: {
        http_req_duration: ['p(95)<2000'],  // Complex query, allow 2s
        errors: ['rate<0.01'],
    },
};

export default function () {
    // Randomly select a template
    const templateId = TEMPLATE_IDS[Math.floor(Math.random() * TEMPLATE_IDS.length)];

    const res = http.get(`${BASE_URL}/api/templates/${templateId}/full`);

    check(res, {
        'status is 200': (r) => r.status === 200,
        'has template data': (r) => {
            const body = JSON.parse(r.body);
            return body.success && body.data.id !== undefined;
        },
        'has days array': (r) => {
            const body = JSON.parse(r.body);
            return Array.isArray(body.data.days);
        },
        'has nested meals': (r) => {
            const body = JSON.parse(r.body);
            return body.data.days.length > 0 &&
                   Array.isArray(body.data.days[0].meals);
        },
    });

    errorRate.add(res.status !== 200);
    latency.add(res.timings.duration);

    sleep(0.2);
}

export function handleSummary(data) {
    return {
        'stdout': JSON.stringify({
            test: 'template-full',
            requests_per_second: data.metrics.http_reqs.values.rate,
            avg_latency_ms: data.metrics.http_req_duration.values.avg,
            p95_latency_ms: data.metrics.http_req_duration.values['p(95)'],
            p99_latency_ms: data.metrics.http_req_duration.values['p(99)'],
            error_rate: data.metrics.errors?.values?.rate || 0,
        }, null, 2),
    };
}
```

#### bulk-insert.js - Write Performance
```javascript
// benchmarks/k6/bulk-insert.js
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Rate, Trend } from 'k6/metrics';

const errorRate = new Rate('errors');
const latency = new Trend('latency');

const BASE_URL = __ENV.BASE_URL || 'http://localhost:3000';

export const options = {
    thresholds: {
        http_req_duration: ['p(95)<3000'],  // Bulk insert can be slow
        errors: ['rate<0.05'],               // Allow higher error rate for writes
    },
};

// Generate random meal items
function generateMealItems(count) {
    const items = [];
    for (let i = 0; i < count; i++) {
        items.push({
            food_item_id: Math.floor(Math.random() * 50) + 1,
            portion_grams_min: 50 + Math.floor(Math.random() * 100),
            portion_grams_max: 150 + Math.floor(Math.random() * 100),
            portion_description: `${Math.floor(Math.random() * 3) + 1} serving(s)`,
            is_optional: Math.random() > 0.7,
            sort_order: i,
        });
    }
    return items;
}

export default function () {
    const payload = JSON.stringify({
        meal_id: Math.floor(Math.random() * 100) + 1,
        items: generateMealItems(50),  // 50 items per request
    });

    const params = {
        headers: {
            'Content-Type': 'application/json',
        },
    };

    const res = http.post(`${BASE_URL}/api/benchmark/bulk-insert`, payload, params);

    check(res, {
        'status is 201': (r) => r.status === 201,
        'items inserted': (r) => {
            const body = JSON.parse(r.body);
            return body.success && body.inserted_count > 0;
        },
    });

    errorRate.add(res.status !== 201);
    latency.add(res.timings.duration);

    sleep(0.5);  // Slower for writes
}

export function handleSummary(data) {
    return {
        'stdout': JSON.stringify({
            test: 'bulk-insert',
            requests_per_second: data.metrics.http_reqs.values.rate,
            avg_latency_ms: data.metrics.http_req_duration.values.avg,
            p95_latency_ms: data.metrics.http_req_duration.values['p(95)'],
            p99_latency_ms: data.metrics.http_req_duration.values['p(99)'],
            error_rate: data.metrics.errors?.values?.rate || 0,
        }, null, 2),
    };
}
```

### Alternative: wrk Script

```bash
#!/bin/bash
# benchmarks/wrk-benchmark.sh

BASE_URL=${1:-http://localhost:3000}
DURATION=${2:-30s}
CONNECTIONS=${3:-100}
THREADS=${4:-4}

echo "=== wrk Benchmark ==="

echo -e "\n[1] GET /api/categories"
wrk -t$THREADS -c$CONNECTIONS -d$DURATION "$BASE_URL/api/categories"

echo -e "\n[2] GET /api/foods?page=1&limit=20"
wrk -t$THREADS -c$CONNECTIONS -d$DURATION "$BASE_URL/api/foods?page=1&limit=20"

echo -e "\n[3] GET /api/templates/1/full"
wrk -t$THREADS -c$CONNECTIONS -d$DURATION "$BASE_URL/api/templates/1/full"
```

---

## 6. Docker Configuration

### Main Docker Compose

```yaml
# docker/docker-compose.yml
version: '3.8'

services:
  # ============================================
  # DATABASES
  # ============================================
  mysql:
    image: mysql:8.0
    container_name: diet-mysql
    environment:
      MYSQL_ROOT_PASSWORD: rootpassword
      MYSQL_DATABASE: diet_benchmark
      MYSQL_USER: benchmark
      MYSQL_PASSWORD: benchmarkpass
    volumes:
      - mysql_data:/var/lib/mysql
      - ../databases/mysql/init.sql:/docker-entrypoint-initdb.d/01-init.sql
      - ../databases/mysql/seed.sql:/docker-entrypoint-initdb.d/02-seed.sql
    ports:
      - "3306:3306"
    healthcheck:
      test: ["CMD", "mysqladmin", "ping", "-h", "localhost"]
      interval: 10s
      timeout: 5s
      retries: 5

  postgres:
    image: postgres:15
    container_name: diet-postgres
    environment:
      POSTGRES_DB: diet_benchmark
      POSTGRES_USER: benchmark
      POSTGRES_PASSWORD: benchmarkpass
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ../databases/postgres/init.sql:/docker-entrypoint-initdb.d/01-init.sql
      - ../databases/postgres/seed.sql:/docker-entrypoint-initdb.d/02-seed.sql
    ports:
      - "5432:5432"
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U benchmark -d diet_benchmark"]
      interval: 10s
      timeout: 5s
      retries: 5

  # ============================================
  # APPLICATIONS
  # ============================================
  nodejs-express:
    build:
      context: ../implementations/nodejs-express
      dockerfile: Dockerfile
    container_name: diet-nodejs
    environment:
      - NODE_ENV=production
      - PORT=3000
      - DB_TYPE=mysql
      - DB_HOST=mysql
      - DB_PORT=3306
      - DB_USER=benchmark
      - DB_PASSWORD=benchmarkpass
      - DB_NAME=diet_benchmark
    ports:
      - "3000:3000"
    depends_on:
      mysql:
        condition: service_healthy

  python-fastapi:
    build:
      context: ../implementations/python-fastapi
      dockerfile: Dockerfile
    container_name: diet-python
    environment:
      - DATABASE_URL=mysql+aiomysql://benchmark:benchmarkpass@mysql:3306/diet_benchmark
    ports:
      - "8000:8000"
    depends_on:
      mysql:
        condition: service_healthy

  go-fiber:
    build:
      context: ../implementations/go-fiber
      dockerfile: Dockerfile
    container_name: diet-go
    environment:
      - DB_DSN=benchmark:benchmarkpass@tcp(mysql:3306)/diet_benchmark
    ports:
      - "8080:8080"
    depends_on:
      mysql:
        condition: service_healthy

  # ============================================
  # BENCHMARK TOOLS
  # ============================================
  k6:
    image: grafana/k6:latest
    container_name: diet-k6
    volumes:
      - ../benchmarks/k6:/scripts
      - ../benchmarks/results:/results
    entrypoint: ["sleep", "infinity"]

volumes:
  mysql_data:
  postgres_data:
```

### Per-Database Overrides

```yaml
# docker/docker-compose.postgres.yml
version: '3.8'

services:
  nodejs-express:
    environment:
      - DB_TYPE=postgres
      - DB_HOST=postgres
      - DB_PORT=5432
    depends_on:
      postgres:
        condition: service_healthy

  python-fastapi:
    environment:
      - DATABASE_URL=postgresql+asyncpg://benchmark:benchmarkpass@postgres:5432/diet_benchmark
    depends_on:
      postgres:
        condition: service_healthy
```

### Example Node.js Dockerfile

```dockerfile
# implementations/nodejs-express/Dockerfile
FROM node:20-alpine

WORKDIR /app

# Install dependencies first (better caching)
COPY package*.json ./
RUN npm ci --only=production

# Copy source
COPY src ./src
COPY .env.example ./.env

# Expose port
EXPOSE 3000

# Health check
HEALTHCHECK --interval=30s --timeout=3s \
    CMD wget --quiet --tries=1 --spider http://localhost:3000/health || exit 1

# Run
CMD ["node", "src/index.js"]
```

### Example Python Dockerfile

```dockerfile
# implementations/python-fastapi/Dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy source
COPY app ./app

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=30s --timeout=3s \
    CMD python -c "import urllib.request; urllib.request.urlopen('http://localhost:8000/health')" || exit 1

# Run with uvicorn
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000", "--workers", "4"]
```

### Example Go Dockerfile

```dockerfile
# implementations/go-fiber/Dockerfile
FROM golang:1.21-alpine AS builder

WORKDIR /app

COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o main .

# Minimal runtime image
FROM alpine:latest

RUN apk --no-cache add ca-certificates
WORKDIR /root/

COPY --from=builder /app/main .

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=3s \
    CMD wget --quiet --tries=1 --spider http://localhost:8080/health || exit 1

CMD ["./main"]
```

---

## 7. VPS Deployment Strategy

### Resource Allocation (2 CPU, 8GB RAM)

**Run benchmarks ONE implementation at a time:**

```
┌─────────────────────────────────────────────────────┐
│ Benchmark Session                                    │
├─────────────────────────────────────────────────────┤
│ Phase 1: Node.js + MySQL                            │
│   - MySQL:     ~500MB                               │
│   - Node.js:   ~150MB                               │
│   - k6:        ~100MB                               │
│   - OS/Docker: ~1GB                                 │
│   - Available: ~6GB buffer                          │
│                                                     │
│ Phase 2: Stop Node.js, Start Python                 │
│   docker-compose stop nodejs-express                │
│   docker-compose up -d python-fastapi               │
│                                                     │
│ Phase 3: Stop Python, Start Go                      │
│   ... and so on                                     │
└─────────────────────────────────────────────────────┘
```

### Benchmark Runner for VPS

```bash
#!/bin/bash
# scripts/run-vps-benchmarks.sh

set -e

RESULTS_DIR="/var/www/diet-benchmark/results/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"

# Function to run benchmark for one implementation
run_benchmark() {
    local impl=$1
    local port=$2
    local db=$3

    echo "=== Benchmarking: $impl with $db ==="

    # Wait for service
    sleep 5
    until curl -s "http://localhost:$port/health" > /dev/null; do
        echo "Waiting for $impl..."
        sleep 2
    done

    # Run k6 benchmarks
    k6 run \
        --vus 50 \
        --duration 30s \
        --out json="$RESULTS_DIR/${impl}_${db}.json" \
        -e BASE_URL="http://localhost:$port" \
        /var/www/diet-benchmark/benchmarks/k6/full-suite.js

    echo "$impl with $db complete!"
}

# ============================================
# MySQL Benchmarks
# ============================================
echo "Starting MySQL..."
docker-compose -f docker-compose.yml up -d mysql
sleep 10  # Wait for MySQL to be ready

# Node.js + MySQL
docker-compose up -d nodejs-express
run_benchmark "nodejs" 3000 "mysql"
docker-compose stop nodejs-express

# Python + MySQL
docker-compose up -d python-fastapi
run_benchmark "python" 8000 "mysql"
docker-compose stop python-fastapi

# Go + MySQL
docker-compose up -d go-fiber
run_benchmark "go" 8080 "mysql"
docker-compose stop go-fiber

# ============================================
# PostgreSQL Benchmarks
# ============================================
docker-compose stop mysql

echo "Starting PostgreSQL..."
docker-compose -f docker-compose.yml -f docker-compose.postgres.yml up -d postgres
sleep 10

# Node.js + PostgreSQL
docker-compose -f docker-compose.yml -f docker-compose.postgres.yml up -d nodejs-express
run_benchmark "nodejs" 3000 "postgres"
docker-compose stop nodejs-express

# Python + PostgreSQL
docker-compose -f docker-compose.yml -f docker-compose.postgres.yml up -d python-fastapi
run_benchmark "python" 8000 "postgres"
docker-compose stop python-fastapi

# Go + PostgreSQL
docker-compose -f docker-compose.yml -f docker-compose.postgres.yml up -d go-fiber
run_benchmark "go" 8080 "postgres"

# ============================================
# Cleanup
# ============================================
docker-compose down

echo "=== All benchmarks complete! ==="
echo "Results saved to: $RESULTS_DIR"
```

### VPS Setup Script

```bash
#!/bin/bash
# scripts/setup-vps.sh

set -e

echo "=== Diet Benchmark VPS Setup ==="

# Update system
sudo apt update && sudo apt upgrade -y

# Install Docker
curl -fsSL https://get.docker.com | sudo sh
sudo usermod -aG docker $USER

# Install Docker Compose
sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose

# Install k6
sudo gpg -k
sudo gpg --no-default-keyring --keyring /usr/share/keyrings/k6-archive-keyring.gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C5AD17C747E3415A3642D57D77C6C491D6AC1D69
echo "deb [signed-by=/usr/share/keyrings/k6-archive-keyring.gpg] https://dl.k6.io/deb stable main" | sudo tee /etc/apt/sources.list.d/k6.list
sudo apt update
sudo apt install k6 -y

# Create project directory
sudo mkdir -p /var/www/diet-benchmark
sudo chown -R $USER:$USER /var/www/diet-benchmark

# Clone repository (update with your repo)
# git clone https://github.com/yourusername/diet-benchmark.git /var/www/diet-benchmark

echo "=== Setup Complete ==="
echo "1. Clone your repository to /var/www/diet-benchmark"
echo "2. Run: cd /var/www/diet-benchmark && docker-compose up -d"
echo "3. Run: ./scripts/run-vps-benchmarks.sh"
```

---

## 8. Results Template

### README.md Results Section

```markdown
# Benchmark Results

**Environment:**
- VPS: Hostinger 2 CPU, 8GB RAM
- OS: Ubuntu 22.04
- Docker: 24.x
- Date: 2024-XX-XX

## Summary

| Implementation | Database | GET /categories | GET /foods | GET /template/full | Bulk Insert |
|----------------|----------|-----------------|------------|--------------------|-------------|
| Node.js Express | MySQL | 2,345 req/s | 1,234 req/s | 456 req/s | 89 req/s |
| Node.js Express | PostgreSQL | 2,567 req/s | 1,345 req/s | 512 req/s | 95 req/s |
| Python FastAPI | MySQL | 1,890 req/s | 987 req/s | 345 req/s | 67 req/s |
| Python FastAPI | PostgreSQL | 2,012 req/s | 1,045 req/s | 389 req/s | 72 req/s |
| Go Fiber | MySQL | 5,678 req/s | 3,456 req/s | 1,234 req/s | 234 req/s |
| Go Fiber | PostgreSQL | 6,123 req/s | 3,789 req/s | 1,456 req/s | 256 req/s |
| C libmicrohttpd | MySQL | 8,901 req/s | 5,678 req/s | 2,345 req/s | 345 req/s |

## Latency (p95)

| Implementation | GET /categories | GET /foods | GET /template/full |
|----------------|-----------------|------------|---------------------|
| Node.js Express | 45ms | 89ms | 234ms |
| Python FastAPI | 52ms | 102ms | 289ms |
| Go Fiber | 18ms | 34ms | 78ms |
| C libmicrohttpd | 12ms | 21ms | 45ms |

## Charts

![Requests per Second](./results/charts/requests-per-second.png)
![Latency Comparison](./results/charts/latency-comparison.png)

## Key Findings

1. **Go outperforms Python/Node.js by 2-3x** for all endpoints
2. **PostgreSQL slightly faster than MySQL** for read-heavy workloads
3. **C implementation is fastest** but with significantly more development effort
4. **Complex joins (template/full)** show the biggest performance gaps
```

### Results JSON Schema

```json
{
  "metadata": {
    "timestamp": "2024-01-15T10:30:00Z",
    "vps": "Hostinger 2CPU/8GB",
    "duration_seconds": 30,
    "connections": 100
  },
  "results": [
    {
      "implementation": "nodejs-express",
      "database": "mysql",
      "tests": {
        "categories": {
          "requests_per_second": 2345,
          "avg_latency_ms": 42.5,
          "p95_latency_ms": 89.2,
          "p99_latency_ms": 145.6,
          "error_rate": 0.001
        },
        "foods_list": {
          "requests_per_second": 1234,
          "avg_latency_ms": 78.3,
          "p95_latency_ms": 156.4,
          "p99_latency_ms": 234.5,
          "error_rate": 0.002
        },
        "template_full": {
          "requests_per_second": 456,
          "avg_latency_ms": 189.5,
          "p95_latency_ms": 345.6,
          "p99_latency_ms": 567.8,
          "error_rate": 0.003
        },
        "bulk_insert": {
          "requests_per_second": 89,
          "avg_latency_ms": 456.7,
          "p95_latency_ms": 789.0,
          "p99_latency_ms": 1234.5,
          "error_rate": 0.01
        }
      }
    }
  ]
}
```

---

## Next Steps

1. **Create GitHub Repository**
   ```bash
   mkdir diet-benchmark
   cd diet-benchmark
   git init
   # Copy this structure
   ```

2. **Implement Node.js API First** (extract from existing project)

3. **Add Python FastAPI Implementation**

4. **Add Go Fiber Implementation**

5. **Run Initial Benchmarks on VPS**

6. **Create Results Charts**

7. **Write LinkedIn Post**

---

## LinkedIn Post Template

```
🚀 Multi-Language API Benchmark: Diet Management System

Just published a performance comparison of the same REST API implemented in:
- Node.js (Express)
- Python (FastAPI)
- Go (Fiber)
- C (libmicrohttpd)

Tested against MySQL & PostgreSQL on a 2-core VPS.

📊 Key findings:
• Go: 3x faster than Python/Node for complex queries
• PostgreSQL: 10-15% faster for reads
• C: Fastest overall, but 10x development time

Full results & code: [GitHub link]

#Backend #Performance #NodeJS #Python #Go #Benchmarking #SoftwareEngineering
```
