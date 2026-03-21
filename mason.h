#ifndef MASON_H
#define MASON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "mason_arena.h"

/* Base Types */

typedef char *string;
typedef cJSON *MASON_Parsed;

extern MASON_Arena *_mason_global_arena;
extern char _mason_path_buffer[256];
extern size_t _mason_path_len;
extern const char *_mason_root_name;

/* Runtime API */
void mason_bind_global_arena(MASON_Arena *a);
void mason_init_default(void);
void mason_init(void);
void mason_reset(void);
void mason_shutdown(void);
const char *mason_error(void);

void _mason_set_parse_error(const char *json_str, const char *err, bool null_input);
void _mason_set_validation_error(const char *name, const char *type_name, const char *resolved_type_name, cJSON *item);
size_t _mason_path_push(const char *name);
size_t _mason_path_push_index(const char *name, size_t index);
void _mason_path_pop(size_t saved);

typedef enum {
    MASON_MISSING,
    MASON_NULL,
    MASON_VALUE,
} MASON_FieldState;

#define _MASON_OPTIONAL_TYPE(type) \
    struct {                       \
        type value;                \
        MASON_FieldState state;    \
    }

typedef _MASON_OPTIONAL_TYPE(int32_t) MASON_Optional_int32_t;
typedef _MASON_OPTIONAL_TYPE(int64_t) MASON_Optional_int64_t;
typedef _MASON_OPTIONAL_TYPE(double) MASON_Optional_double;
typedef _MASON_OPTIONAL_TYPE(string) MASON_Optional_string;
typedef _MASON_OPTIONAL_TYPE(bool) MASON_Optional_bool;

#define _MASON_OPTIONAL_ARRAY_TYPE(type) \
    struct {                             \
        type *items;                     \
        size_t count;                    \
        MASON_FieldState state;          \
    }

typedef _MASON_OPTIONAL_ARRAY_TYPE(int32_t) MASON_OptionalArray_int32_t;
typedef _MASON_OPTIONAL_ARRAY_TYPE(int64_t) MASON_OptionalArray_int64_t;
typedef _MASON_OPTIONAL_ARRAY_TYPE(double) MASON_OptionalArray_double;
typedef _MASON_OPTIONAL_ARRAY_TYPE(string) MASON_OptionalArray_string;
typedef _MASON_OPTIONAL_ARRAY_TYPE(bool) MASON_OptionalArray_bool;

#define mason_opt_value(v)                {.value = v, .state = MASON_VALUE}
#define mason_opt_null()                  {.state = MASON_NULL}
#define mason_opt_missing()               {.state = MASON_MISSING}
#define mason_opt_array(items_ptr, cnt)   {.items = items_ptr, .count = cnt, .state = MASON_VALUE}

/* Field Type Macros */

#define _MASON_FIELD_REQUIRED(type, name) type name;
#define _MASON_FIELD_OPTIONAL(type, name) MASON_Optional_##type name;
#define _MASON_FIELD_NULLABLE(type, name) MASON_Optional_##type name;
#define _MASON_FIELD(type, name, qual)    _MASON_CONCAT(_MASON_FIELD_, qual)(type, name)

#define _MASON_ARRAY_REQUIRED(type, name) \
    type *name;                           \
    size_t name##_count;
#define _MASON_ARRAY_NULLABLE(type, name)  MASON_OptionalArray_##type name;
#define _MASON_ARRAY_OPTIONAL(type, name)  MASON_OptionalArray_##type name;
#define _MASON_ARRAY(type, name, qual)     _MASON_CONCAT(_MASON_ARRAY_, qual)(type, name)

#define _MASON_OBJECT_REQUIRED(type, name) struct type *name;
#define _MASON_OBJECT_NULLABLE(type, name) MASON_Optional_##type name;
#define _MASON_OBJECT_OPTIONAL(type, name) MASON_Optional_##type name;
#define _MASON_OBJECT(type, name, qual)    _MASON_CONCAT(_MASON_OBJECT_, qual)(type, name)

#define _MASON_ARRAY_OBJECT_REQUIRED(type, name) \
    type *name;                                  \
    size_t name##_count;
#define _MASON_ARRAY_OBJECT_NULLABLE(type, name) MASON_OptionalArray_##type name;
#define _MASON_ARRAY_OBJECT_OPTIONAL(type, name) MASON_OptionalArray_##type name;
#define _MASON_ARRAY_OBJECT(type, name, qual)    _MASON_CONCAT(_MASON_ARRAY_OBJECT_, qual)(type, name)

/* Struct Definition */

#define MASON_STRUCT_DEFINE(struct_name, FIELDS)                                       \
    typedef struct struct_name {                                                       \
        FIELDS(_MASON_FIELD, _MASON_ARRAY, _MASON_OBJECT, _MASON_ARRAY_OBJECT)         \
    } struct_name;                                                                     \
                                                                                       \
    typedef _MASON_OPTIONAL_TYPE(struct_name) MASON_Optional_##struct_name;            \
    typedef _MASON_OPTIONAL_ARRAY_TYPE(struct_name) MASON_OptionalArray_##struct_name; \
                                                                                       \
    bool struct_name##_parse_into(struct_name *obj, MASON_Parsed json);                \
    struct_name *struct_name##_from_json(MASON_Parsed json);                           \
    struct_name *struct_name##_from_string(const char *json_str);                      \
    struct_name *struct_name##_from_string_sized(const char *json_str, size_t len);    \
    MASON_Parsed struct_name##_to_json(struct_name *obj);                              \
    string struct_name##_to_string(MASON_Parsed json, bool format);

/* Type Resolution
 *
 * Allows type aliases with a single #define:
 *   #define MASON_TYPE_ALIAS_MyType int32_t
 *
 * Primitives have identity mappings so they resolve to themselves.
 */

#define _MASON_CONCAT_INNER(a, b) a##b
#define _MASON_CONCAT(a, b)       _MASON_CONCAT_INNER(a, b)
#define _MASON_TYPE_ALIAS(type)   _MASON_CONCAT(MASON_TYPE_ALIAS_, type)
#define _MASON_STRINGIFY_INNER(x) #x
#define _MASON_STRINGIFY(x)       _MASON_STRINGIFY_INNER(x)
#define MASON_TYPE_NAME(type)     _MASON_STRINGIFY(_MASON_TYPE_ALIAS(type))
#define MASON_TYPE_HINT(type)     ((_MASON_TYPE_ALIAS(type))0)

#define MASON_TYPE_ALIAS_int32_t  int32_t
#define MASON_TYPE_ALIAS_int64_t  int64_t
#define MASON_TYPE_ALIAS_double   double
#define MASON_TYPE_ALIAS_string   string
#define MASON_TYPE_ALIAS_bool     bool
#define MASON_TYPE_ALIAS__Bool    bool

/* Inline Type Helpers */

/* Type checkers */
static inline bool mason_is_int32(MASON_Parsed item) { return cJSON_IsNumber(item); }
static inline bool mason_is_int64(MASON_Parsed item) { return cJSON_IsNumber(item); }
static inline bool mason_is_double(MASON_Parsed item) { return cJSON_IsNumber(item); }
static inline bool mason_is_string(MASON_Parsed item) { return cJSON_IsString(item) && item->valuestring; }
static inline bool mason_is_bool(MASON_Parsed item) { return cJSON_IsBool(item); }

/* Non-owning value getters */
static inline int32_t mason_get_int32(MASON_Parsed item) { return (int32_t)item->valueint; }
static inline int64_t mason_get_int64(MASON_Parsed item) { return (int64_t)item->valuedouble; }
static inline double mason_get_double(MASON_Parsed item) { return item->valuedouble; }
static inline const char *mason_get_string(MASON_Parsed item) { return item->valuestring; }
static inline bool mason_get_bool(MASON_Parsed item) { return cJSON_IsTrue(item); }

/* Owning getters
 * NOTE: strdup for strings, passthrough for primitives
 */
static inline int32_t mason_get_owned_int32(MASON_Parsed item) { return (int32_t)item->valueint; }
static inline int64_t mason_get_owned_int64(MASON_Parsed item) { return (int64_t)item->valuedouble; }
static inline double mason_get_owned_double(MASON_Parsed item) { return item->valuedouble; }
static inline char *mason_get_owned_string(MASON_Parsed item) {
    if (!item || !item->valuestring) {
        return NULL;
    }
    size_t len = strlen(item->valuestring) + 1;
    char *p = (char *)mason_arena_alloc(_mason_global_arena, len);
    if (p) {
        memcpy(p, item->valuestring, len);
    }
    return p;
}
static inline bool mason_get_owned_bool(MASON_Parsed item) { return cJSON_IsTrue(item); }

/* JSON node creators */
static inline MASON_Parsed mason_create_int32(int32_t v) { return cJSON_CreateNumber(v); }
static inline MASON_Parsed mason_create_int64(int64_t v) { return cJSON_CreateNumber((double)v); }
static inline MASON_Parsed mason_create_double(double v) { return cJSON_CreateNumber(v); }
static inline MASON_Parsed mason_create_string(const char *v) { return v ? cJSON_CreateString(v) : cJSON_CreateNull(); }
static inline MASON_Parsed mason_create_bool(bool v) { return cJSON_CreateBool(v); }

/* _Generic Dispatch Macros */

#define mason_is(item, type_hint) _Generic((type_hint), \
    int32_t: mason_is_int32,                            \
    int64_t: mason_is_int64,                            \
    double: mason_is_double,                            \
    char *: mason_is_string,                            \
    _Bool: mason_is_bool)(item)

#define mason_get(item, type_hint) _Generic((type_hint), \
    int32_t: mason_get_int32,                            \
    int64_t: mason_get_int64,                            \
    double: mason_get_double,                            \
    char *: mason_get_string,                            \
    _Bool: mason_get_bool)(item)

#define mason_get_owned(item, type_hint) _Generic((type_hint), \
    int32_t: mason_get_owned_int32,                            \
    int64_t: mason_get_owned_int64,                            \
    double: mason_get_owned_double,                            \
    char *: mason_get_owned_string,                            \
    _Bool: mason_get_owned_bool)(item)

#define mason_create(value) _Generic((value), \
    int32_t: mason_create_int32,              \
    int64_t: mason_create_int64,              \
    double: mason_create_double,              \
    char *: mason_create_string,              \
    const char *: mason_create_string,        \
    _Bool: mason_create_bool)(value)

/*
 * Parsing Implementation
 * Arguments that come from the functions these macros are expanded in:
 * `json` - the JSON object being parsed
 * `obj`  - the struct being parsed into
 * `item` - temporary cJSON handle for the field being parsed
 */

#define _MASON_PARSE_FIELD_REQUIRED(type, name)                                 \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                       \
    if (item && mason_is(item, MASON_TYPE_HINT(type))) {                        \
        obj->name = mason_get_owned(item, MASON_TYPE_HINT(type));               \
    } else {                                                                    \
        _mason_set_validation_error(#name, #type, MASON_TYPE_NAME(type), item); \
        return false;                                                           \
    }
#define _MASON_PARSE_FIELD_NULLABLE(type, name)                                                       \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                                             \
    if (item && mason_is(item, MASON_TYPE_HINT(type))) {                                              \
        obj->name.value = mason_get_owned(item, MASON_TYPE_HINT(type));                               \
        obj->name.state = MASON_VALUE;                                                                \
    } else if (item && cJSON_IsNull(item)) {                                                          \
        obj->name.state = MASON_NULL;                                                                 \
    } else {                                                                                          \
        _mason_set_validation_error(#name, #type " or null", MASON_TYPE_NAME(type) " or null", item); \
        return false;                                                                                 \
    }
#define _MASON_PARSE_FIELD_OPTIONAL(type, name)                         \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);               \
    if (item && mason_is(item, MASON_TYPE_HINT(type))) {                \
        obj->name.value = mason_get_owned(item, MASON_TYPE_HINT(type)); \
        obj->name.state = MASON_VALUE;                                  \
    } else if (item && cJSON_IsNull(item)) {                            \
        obj->name.state = MASON_NULL;                                   \
    } else {                                                            \
        obj->name.state = MASON_MISSING;                                \
    }

#define _MASON_PARSE_ARRAY_PRIM_BODY(type, name_str, arr_ptr, arr_count)                    \
    arr_count = (size_t)cJSON_GetArraySize(item);                                           \
    if (arr_count == 0) {                                                                   \
        arr_ptr = NULL;                                                                     \
    } else {                                                                                \
        MASON_ArenaMark mark = mason_arena_mark(_mason_global_arena);                       \
        arr_ptr = (type *)mason_arena_calloc(_mason_global_arena, arr_count, sizeof(type)); \
        if (!arr_ptr) {                                                                     \
            arr_count = 0;                                                                  \
            return false;                                                                   \
        }                                                                                   \
        bool _arr_ok = true;                                                                \
        for (size_t i = 0; i < arr_count; i++) {                                            \
            MASON_Parsed elem = cJSON_GetArrayItem(item, (int)i);                           \
            if (mason_is(elem, MASON_TYPE_HINT(type))) {                                    \
                arr_ptr[i] = mason_get_owned(elem, MASON_TYPE_HINT(type));                  \
            } else {                                                                        \
                mason_arena_rewind(_mason_global_arena, mark);                              \
                arr_ptr = NULL;                                                             \
                arr_count = 0;                                                              \
                _arr_ok = false;                                                            \
                _mason_set_validation_error(name_str, #type, MASON_TYPE_NAME(type), elem);  \
                break;                                                                      \
            }                                                                               \
        }                                                                                   \
        if (!_arr_ok)                                                                       \
            return false;                                                                   \
    }

#define _MASON_PARSE_ARRAY_PRIM_REQUIRED(type, name)                                      \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                                 \
    if (item && cJSON_IsArray(item)) {                                                    \
        _MASON_PARSE_ARRAY_PRIM_BODY(type, #name, obj->name, obj->name##_count)           \
    } else {                                                                              \
        _mason_set_validation_error(#name, #type "[]", MASON_TYPE_NAME(type) "[]", item); \
        return false;                                                                     \
    }
#define _MASON_PARSE_ARRAY_PRIM_NULLABLE(type, name)                                                      \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                                                 \
    if (item && cJSON_IsArray(item)) {                                                                    \
        _MASON_PARSE_ARRAY_PRIM_BODY(type, #name, obj->name.items, obj->name.count)                       \
        obj->name.state = MASON_VALUE;                                                                    \
    } else if (item && cJSON_IsNull(item)) {                                                              \
        obj->name.state = MASON_NULL;                                                                     \
    } else {                                                                                              \
        _mason_set_validation_error(#name, #type "[] or null", MASON_TYPE_NAME(type) "[] or null", item); \
        return false;                                                                                     \
    }
#define _MASON_PARSE_ARRAY_PRIM_OPTIONAL(type, name)                                \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                           \
    if (item && cJSON_IsArray(item)) {                                              \
        _MASON_PARSE_ARRAY_PRIM_BODY(type, #name, obj->name.items, obj->name.count) \
        obj->name.state = MASON_VALUE;                                              \
    } else if (item && cJSON_IsNull(item)) {                                        \
        obj->name.state = MASON_NULL;                                               \
    } else {                                                                        \
        obj->name.state = MASON_MISSING;                                            \
    }

#define _MASON_PARSE_OBJECT_REQUIRED(type, name)                                      \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                             \
    if (item && cJSON_IsObject(item)) {                                               \
        MASON_ArenaMark mark = mason_arena_mark(_mason_global_arena);                 \
        obj->name = (type *)mason_arena_calloc(_mason_global_arena, 1, sizeof(type)); \
        if (!obj->name) {                                                             \
            return false;                                                             \
        }                                                                             \
        size_t _path_saved = _mason_path_push(#name);                                 \
        bool _ok = type##_parse_into(obj->name, item);                                \
        _mason_path_pop(_path_saved);                                                 \
        if (!_ok) {                                                                   \
            mason_arena_rewind(_mason_global_arena, mark);                            \
            obj->name = NULL;                                                         \
            return false;                                                             \
        }                                                                             \
    } else {                                                                          \
        _mason_set_validation_error(#name, #type, "object", item);                    \
        return false;                                                                 \
    }
#define _MASON_PARSE_OBJECT_NULLABLE(type, name)                                      \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                             \
    if (item && cJSON_IsObject(item)) {                                               \
        size_t _path_saved = _mason_path_push(#name);                                 \
        bool _ok = type##_parse_into(&obj->name.value, item);                         \
        _mason_path_pop(_path_saved);                                                 \
        if (!_ok) {                                                                   \
            memset(&obj->name.value, 0, sizeof(type));                                \
            return false;                                                             \
        }                                                                             \
        obj->name.state = MASON_VALUE;                                                \
    } else if (item && cJSON_IsNull(item)) {                                          \
        obj->name.state = MASON_NULL;                                                 \
    } else {                                                                          \
        _mason_set_validation_error(#name, #type " or null", "object or null", item); \
        return false;                                                                 \
    }
#define _MASON_PARSE_OBJECT_OPTIONAL(type, name)              \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);     \
    if (item && cJSON_IsObject(item)) {                       \
        size_t _path_saved = _mason_path_push(#name);         \
        bool _ok = type##_parse_into(&obj->name.value, item); \
        _mason_path_pop(_path_saved);                         \
        if (!_ok) {                                           \
            memset(&obj->name.value, 0, sizeof(type));        \
            return false;                                     \
        }                                                     \
        obj->name.state = MASON_VALUE;                        \
    } else if (item && cJSON_IsNull(item)) {                  \
        obj->name.state = MASON_NULL;                         \
    } else {                                                  \
        obj->name.state = MASON_MISSING;                      \
    }

#define _MASON_PARSE_ARRAY_OBJECT_BODY(type, name_str, arr_ptr, arr_count)                  \
    arr_count = (size_t)cJSON_GetArraySize(item);                                           \
    if (arr_count == 0) {                                                                   \
        arr_ptr = NULL;                                                                     \
    } else {                                                                                \
        MASON_ArenaMark mark = mason_arena_mark(_mason_global_arena);                       \
        arr_ptr = (type *)mason_arena_calloc(_mason_global_arena, arr_count, sizeof(type)); \
        if (!arr_ptr) {                                                                     \
            arr_count = 0;                                                                  \
            return false;                                                                   \
        }                                                                                   \
        for (size_t i = 0; i < arr_count; i++) {                                            \
            MASON_Parsed elem = cJSON_GetArrayItem(item, (int)i);                           \
            size_t _path_saved = _mason_path_push_index(name_str, i);                       \
            bool _ok = type##_parse_into(&arr_ptr[i], elem);                                \
            _mason_path_pop(_path_saved);                                                   \
            if (!_ok) {                                                                     \
                mason_arena_rewind(_mason_global_arena, mark);                              \
                arr_ptr = NULL;                                                             \
                arr_count = 0;                                                              \
                return false;                                                               \
            }                                                                               \
        }                                                                                   \
    }

#define _MASON_PARSE_ARRAY_OBJECT_REQUIRED(type, name)                            \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                         \
    if (item && cJSON_IsArray(item)) {                                            \
        _MASON_PARSE_ARRAY_OBJECT_BODY(type, #name, obj->name, obj->name##_count) \
    } else {                                                                      \
        _mason_set_validation_error(#name, #type "[]", "object array", item);     \
        return false;                                                             \
    }
#define _MASON_PARSE_ARRAY_OBJECT_NULLABLE(type, name)                                        \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                                     \
    if (item && cJSON_IsArray(item)) {                                                        \
        _MASON_PARSE_ARRAY_OBJECT_BODY(type, #name, obj->name.items, obj->name.count)         \
        obj->name.state = MASON_VALUE;                                                        \
    } else if (item && cJSON_IsNull(item)) {                                                  \
        obj->name.state = MASON_NULL;                                                         \
    } else {                                                                                  \
        _mason_set_validation_error(#name, #type "[] or null", "object array or null", item); \
        return false;                                                                         \
    }
#define _MASON_PARSE_ARRAY_OBJECT_OPTIONAL(type, name)                                \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                             \
    if (item && cJSON_IsArray(item)) {                                                \
        _MASON_PARSE_ARRAY_OBJECT_BODY(type, #name, obj->name.items, obj->name.count) \
        obj->name.state = MASON_VALUE;                                                \
    } else if (item && cJSON_IsNull(item)) {                                          \
        obj->name.state = MASON_NULL;                                                 \
    } else {                                                                          \
        obj->name.state = MASON_MISSING;                                              \
    }

/* Serialization Implementation */

#define _MASON_SERIALIZE_FIELD_REQUIRED(type, name) \
    cJSON_AddItemToObject(json, #name, mason_create((_MASON_TYPE_ALIAS(type))obj->name));
#define _MASON_SERIALIZE_FIELD_NULLABLE(type, name)                                                 \
    switch (obj->name.state) {                                                                      \
    case MASON_VALUE:                                                                               \
        cJSON_AddItemToObject(json, #name, mason_create((_MASON_TYPE_ALIAS(type))obj->name.value)); \
        break;                                                                                      \
    case MASON_NULL:                                                                                \
    case MASON_MISSING:                                                                             \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                                     \
        break;                                                                                      \
    }
#define _MASON_SERIALIZE_FIELD_OPTIONAL(type, name)                                                 \
    switch (obj->name.state) {                                                                      \
    case MASON_VALUE:                                                                               \
        cJSON_AddItemToObject(json, #name, mason_create((_MASON_TYPE_ALIAS(type))obj->name.value)); \
        break;                                                                                      \
    case MASON_NULL:                                                                                \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                                     \
        break;                                                                                      \
    case MASON_MISSING:                                                                             \
        break;                                                                                      \
    }

#define _MASON_SERIALIZE_ARRAY_PRIM_BODY(type, name_str, arr_ptr, arr_count)         \
    {                                                                                \
        MASON_Parsed arr = cJSON_CreateArray();                                      \
        for (size_t i = 0; i < arr_count; i++) {                                     \
            cJSON_AddItemToArray(arr,                                                \
                                 mason_create((_MASON_TYPE_ALIAS(type))arr_ptr[i])); \
        }                                                                            \
        cJSON_AddItemToObject(json, name_str, arr);                                  \
    }

#define _MASON_SERIALIZE_ARRAY_PRIM_REQUIRED(type, name) \
    _MASON_SERIALIZE_ARRAY_PRIM_BODY(type, #name, obj->name, obj->name##_count)
#define _MASON_SERIALIZE_ARRAY_PRIM_NULLABLE(type, name)                                \
    switch (obj->name.state) {                                                          \
    case MASON_VALUE:                                                                   \
        _MASON_SERIALIZE_ARRAY_PRIM_BODY(type, #name, obj->name.items, obj->name.count) \
        break;                                                                          \
    case MASON_NULL:                                                                    \
    case MASON_MISSING:                                                                 \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                         \
        break;                                                                          \
    }
#define _MASON_SERIALIZE_ARRAY_PRIM_OPTIONAL(type, name)                                \
    switch (obj->name.state) {                                                          \
    case MASON_VALUE:                                                                   \
        _MASON_SERIALIZE_ARRAY_PRIM_BODY(type, #name, obj->name.items, obj->name.count) \
        break;                                                                          \
    case MASON_NULL:                                                                    \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                         \
        break;                                                                          \
    case MASON_MISSING:                                                                 \
        break;                                                                          \
    }

#define _MASON_SERIALIZE_OBJECT_REQUIRED(type, name)     \
    if (obj->name) {                                     \
        MASON_Parsed nested = type##_to_json(obj->name); \
        if (nested) {                                    \
            cJSON_AddItemToObject(json, #name, nested);  \
        }                                                \
    }
#define _MASON_SERIALIZE_OBJECT_NULLABLE(type, name)            \
    switch (obj->name.state) {                                  \
    case MASON_VALUE: {                                         \
        MASON_Parsed nested = type##_to_json(&obj->name.value); \
        if (nested) {                                           \
            cJSON_AddItemToObject(json, #name, nested);         \
        }                                                       \
        break;                                                  \
    }                                                           \
    case MASON_NULL:                                            \
    case MASON_MISSING:                                         \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull()); \
        break;                                                  \
    }
#define _MASON_SERIALIZE_OBJECT_OPTIONAL(type, name)            \
    switch (obj->name.state) {                                  \
    case MASON_VALUE: {                                         \
        MASON_Parsed nested = type##_to_json(&obj->name.value); \
        if (nested) {                                           \
            cJSON_AddItemToObject(json, #name, nested);         \
        }                                                       \
        break;                                                  \
    }                                                           \
    case MASON_NULL:                                            \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull()); \
        break;                                                  \
    case MASON_MISSING:                                         \
        break;                                                  \
    }

#define _MASON_SERIALIZE_ARRAY_OBJECT_BODY(type, name_str, arr_ptr, arr_count) \
    {                                                                          \
        MASON_Parsed arr = cJSON_CreateArray();                                \
        for (size_t i = 0; i < arr_count; i++) {                               \
            MASON_Parsed nested = type##_to_json(&arr_ptr[i]);                 \
            if (nested) {                                                      \
                cJSON_AddItemToArray(arr, nested);                             \
            }                                                                  \
        }                                                                      \
        cJSON_AddItemToObject(json, name_str, arr);                            \
    }

#define _MASON_SERIALIZE_ARRAY_OBJECT_REQUIRED(type, name) \
    _MASON_SERIALIZE_ARRAY_OBJECT_BODY(type, #name, obj->name, obj->name##_count)
#define _MASON_SERIALIZE_ARRAY_OBJECT_NULLABLE(type, name)                                \
    switch (obj->name.state) {                                                            \
    case MASON_VALUE:                                                                     \
        _MASON_SERIALIZE_ARRAY_OBJECT_BODY(type, #name, obj->name.items, obj->name.count) \
        break;                                                                            \
    case MASON_NULL:                                                                      \
    case MASON_MISSING:                                                                   \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                           \
        break;                                                                            \
    }
#define _MASON_SERIALIZE_ARRAY_OBJECT_OPTIONAL(type, name)                                \
    switch (obj->name.state) {                                                            \
    case MASON_VALUE:                                                                     \
        _MASON_SERIALIZE_ARRAY_OBJECT_BODY(type, #name, obj->name.items, obj->name.count) \
        break;                                                                            \
    case MASON_NULL:                                                                      \
        cJSON_AddItemToObject(json, #name, cJSON_CreateNull());                           \
        break;                                                                            \
    case MASON_MISSING:                                                                   \
        break;                                                                            \
    }

/* X-Macro Expansion Helpers */

#define _MASON_EXPAND_PARSE_FIELD(type, name, qual)            _MASON_CONCAT(_MASON_PARSE_FIELD_, qual)(type, name)
#define _MASON_EXPAND_PARSE_ARRAY(type, name, qual)            _MASON_CONCAT(_MASON_PARSE_ARRAY_PRIM_, qual)(type, name)
#define _MASON_EXPAND_PARSE_OBJECT(type, name, qual)           _MASON_CONCAT(_MASON_PARSE_OBJECT_, qual)(type, name)
#define _MASON_EXPAND_PARSE_ARRAY_OBJECT(type, name, qual)     _MASON_CONCAT(_MASON_PARSE_ARRAY_OBJECT_, qual)(type, name)

#define _MASON_EXPAND_SERIALIZE_FIELD(type, name, qual)        _MASON_CONCAT(_MASON_SERIALIZE_FIELD_, qual)(type, name)
#define _MASON_EXPAND_SERIALIZE_ARRAY(type, name, qual)        _MASON_CONCAT(_MASON_SERIALIZE_ARRAY_PRIM_, qual)(type, name)
#define _MASON_EXPAND_SERIALIZE_OBJECT(type, name, qual)       _MASON_CONCAT(_MASON_SERIALIZE_OBJECT_, qual)(type, name)
#define _MASON_EXPAND_SERIALIZE_ARRAY_OBJECT(type, name, qual) _MASON_CONCAT(_MASON_SERIALIZE_ARRAY_OBJECT_, qual)(type, name)

/* Main Implementation Macros */

#define _MASON_IMPL_BASE(struct_name, FIELDS)                                               \
    bool struct_name##_parse_into(struct_name *obj, MASON_Parsed json) {                    \
        if (!obj || !json || !cJSON_IsObject(json)) {                                       \
            return false;                                                                   \
        }                                                                                   \
        MASON_Parsed item = NULL;                                                           \
        FIELDS(_MASON_EXPAND_PARSE_FIELD,                                                   \
               _MASON_EXPAND_PARSE_ARRAY,                                                   \
               _MASON_EXPAND_PARSE_OBJECT,                                                  \
               _MASON_EXPAND_PARSE_ARRAY_OBJECT)                                            \
        return true;                                                                        \
    }                                                                                       \
                                                                                            \
    struct_name *struct_name##_from_json(MASON_Parsed json) {                               \
        if (!json) {                                                                        \
            return NULL;                                                                    \
        }                                                                                   \
        _mason_path_len = 0;                                                                \
        _mason_path_buffer[0] = '\0';                                                       \
        _mason_root_name = #struct_name;                                                    \
        MASON_ArenaMark mark = mason_arena_mark(_mason_global_arena);                       \
        struct_name *obj = mason_arena_calloc(_mason_global_arena, 1, sizeof(struct_name)); \
        if (!obj) {                                                                         \
            return NULL;                                                                    \
        }                                                                                   \
        if (!struct_name##_parse_into(obj, json)) {                                         \
            mason_arena_rewind(_mason_global_arena, mark);                                  \
            return NULL;                                                                    \
        }                                                                                   \
        return obj;                                                                         \
    }                                                                                       \
                                                                                            \
    struct_name *struct_name##_from_string(const char *json_str) {                          \
        if (!json_str) {                                                                    \
            _mason_set_parse_error(NULL, NULL, true);                                       \
            return NULL;                                                                    \
        }                                                                                   \
        const char *parse_end = NULL;                                                       \
        MASON_Parsed parsed = cJSON_ParseWithOpts(json_str, &parse_end, 0);                 \
        if (!parsed) {                                                                      \
            _mason_set_parse_error(json_str, parse_end, false);                             \
            return NULL;                                                                    \
        }                                                                                   \
        _mason_set_parse_error(NULL, NULL, false);                                          \
        struct_name *obj = struct_name##_from_json(parsed);                                 \
        return obj;                                                                         \
    }                                                                                       \
                                                                                            \
    struct_name *struct_name##_from_string_sized(const char *json_str, size_t len) {        \
        if (!json_str) {                                                                    \
            _mason_set_parse_error(NULL, NULL, true);                                       \
            return NULL;                                                                    \
        }                                                                                   \
        const char *parse_end = NULL;                                                       \
        MASON_Parsed parsed = cJSON_ParseWithLengthOpts(json_str, len, &parse_end, 0);      \
        if (!parsed) {                                                                      \
            _mason_set_parse_error(json_str, parse_end, false);                             \
            return NULL;                                                                    \
        }                                                                                   \
        _mason_set_parse_error(NULL, NULL, false);                                          \
        struct_name *obj = struct_name##_from_json(parsed);                                 \
        return obj;                                                                         \
    }                                                                                       \
                                                                                            \
    MASON_Parsed struct_name##_to_json(struct_name *obj) {                                  \
        if (!obj) {                                                                         \
            return NULL;                                                                    \
        }                                                                                   \
        MASON_Parsed json = cJSON_CreateObject();                                           \
        if (!json) {                                                                        \
            return NULL;                                                                    \
        }                                                                                   \
        FIELDS(_MASON_EXPAND_SERIALIZE_FIELD, _MASON_EXPAND_SERIALIZE_ARRAY,                \
               _MASON_EXPAND_SERIALIZE_OBJECT, _MASON_EXPAND_SERIALIZE_ARRAY_OBJECT)        \
        return json;                                                                        \
    }                                                                                       \
                                                                                            \
    string struct_name##_to_string(MASON_Parsed json, bool format) {                        \
        if (!json) {                                                                        \
            return NULL;                                                                    \
        }                                                                                   \
        if (format) {                                                                       \
            return cJSON_Print(json);                                                       \
        } else {                                                                            \
            return cJSON_PrintUnformatted(json);                                            \
        }                                                                                   \
    }

#define MASON_IMPL(struct_name, FIELDS) \
    _MASON_IMPL_BASE(struct_name, FIELDS)

#endif // MASON_H
