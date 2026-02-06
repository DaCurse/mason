#ifndef MASON_H
#define MASON_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef MASON_PRINT_IMPL
#include <stdio.h>
#endif
#ifndef MASON_PRINT_IMPL
#include <stdio.h>
#endif

#ifdef MASON_USE_CJSON
#include "backends/cjson.h"
#endif

#ifdef _MASON_BACKEND

static char *mason_strdup(const char *s) {
    if (!s)
        return NULL;
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p)
        memcpy(p, s, len);
    return p;
}

/* Type Definitions */

typedef char *string;
typedef _MASON_BACKEND_JSON_T MASON_Parsed;

/* Helpers */

static inline MASON_Parsed *MASON_Parse(const char *json_str) {
    if (!json_str)
        return NULL;
    return _MASON_BACKEND_PARSE(json_str);
}

static inline MASON_Parsed *MASON_ParseSized(const char *json_str, size_t len) {
    if (!json_str)
        return NULL;
    return _MASON_BACKEND_PARSE_SIZED(json_str, len);
}

static inline const char *MASON_ParseError(void) {
    return _MASON_BACKEND_PARSE_ERROR();
}

#include "mason_multi.h"

/* Field Type Macros */

#define MASON_FIELD(type, name) type name;
#define MASON_ARRAY(type, name) \
    type *name;                 \
    size_t name##_count;
#define MASON_OBJECT(type, name) struct type *name;
#define MASON_ARRAY_OBJECT(type, name) \
    type *name;                        \
    size_t name##_count;

/* Struct Definition */

#define MASON_STRUCT_DEFINE(struct_name, FIELDS)                                                  \
    typedef struct struct_name {                                                                  \
        FIELDS(MASON_FIELD, MASON_ARRAY, MASON_ARRAY_MULTI_RAW, MASON_OBJECT, MASON_ARRAY_OBJECT) \
    } struct_name;                                                                                \
                                                                                                  \
    struct_name *struct_name##_from_parsed(MASON_Parsed *json);                                   \
    struct_name *struct_name##_from_json(const char *json_str);                                   \
    struct_name *struct_name##_from_json_sized(const char *json_str, size_t len);                 \
    MASON_Parsed *struct_name##_to_json(struct_name *obj);                                        \
    void struct_name##_free(struct_name *obj);                                                    \
    void struct_name##_free_members(struct_name *obj);                                            \
    void struct_name##_free_json(MASON_Parsed *json);                                             \
    string struct_name##_to_string(MASON_Parsed *json);                                           \
    void struct_name##_string_free(string str);                                                   \
    void struct_name##_print(struct_name *obj);

/* Type Resolution
 *
 * Allows type aliases with a single #define:
 *   #define MASON_BASE_TYPE_MyType int32_t
 *
 * Primitives have identity mappings so they resolve to themselves.
 */

#define _MASON_CONCAT_INNER(a, b)     a##b
#define _MASON_CONCAT(a, b)           _MASON_CONCAT_INNER(a, b)
#define _MASON_RESOLVE(type)          MASON_BASE_TYPE_##type

#define MASON_BASE_TYPE_int32_t       int32_t
#define MASON_BASE_TYPE_int64_t       int64_t
#define MASON_BASE_TYPE_double        double
#define MASON_BASE_TYPE_string        string
#define MASON_BASE_TYPE_bool          bool
#define MASON_BASE_TYPE__Bool         bool

/* Format Strings */

#define _MASON_FMT_int32_t            "%" PRId32
#define _MASON_FMT_int64_t            "%" PRId64
#define _MASON_FMT_double             "%f"
#define _MASON_FMT_string             "\"%s\""
#define _MASON_FMT_bool               "%s"

/* Parser Implementation */

#define MASON_PARSE_FIELD(type, name) _MASON_CONCAT(MASON_PARSE_FIELD_, _MASON_RESOLVE(type))(name)

#define _MASON_TRANSFORM_NOOP(elem)   elem
#define _MASON_TRANSFORM_STRDUP(elem) mason_strdup(elem)

#define _MASON_PARSE_FIELD_GENERIC(name, type, transform)       \
    item = _MASON_BACKEND_GET_FIELD(json, #name);               \
    if (_MASON_BACKEND_IS_##type(item)) {                       \
        obj->name = transform(_MASON_BACKEND_GET_##type(item)); \
    }

#define MASON_PARSE_FIELD_int32_t(name) _MASON_PARSE_FIELD_GENERIC(name, int32_t, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_FIELD_int64_t(name) _MASON_PARSE_FIELD_GENERIC(name, int64_t, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_FIELD_double(name)  _MASON_PARSE_FIELD_GENERIC(name, double, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_FIELD_string(name)  _MASON_PARSE_FIELD_GENERIC(name, string, _MASON_TRANSFORM_STRDUP)
#define MASON_PARSE_FIELD_bool(name)    _MASON_PARSE_FIELD_GENERIC(name, bool, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_FIELD__Bool(name)   MASON_PARSE_FIELD_bool(name)

#define _MASON_PARSE_ARRAY_GENERIC(name, type, transform)                  \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                          \
    if (_MASON_BACKEND_IS_ARRAY(item)) {                                   \
        obj->name##_count = _MASON_BACKEND_ARRAY_SIZE(item);               \
        obj->name = (type *)calloc(obj->name##_count, sizeof(type));       \
        for (size_t i = 0; i < obj->name##_count; i++) {                   \
            MASON_Parsed *elem = _MASON_BACKEND_ARRAY_GET(item, i);        \
            if (_MASON_BACKEND_IS_##type(elem)) {                          \
                obj->name[i] = transform(_MASON_BACKEND_GET_##type(elem)); \
            }                                                              \
        }                                                                  \
    }

#define MASON_PARSE_ARRAY_int32_t(name) _MASON_PARSE_ARRAY_GENERIC(name, int32_t, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_ARRAY_int64_t(name) _MASON_PARSE_ARRAY_GENERIC(name, int64_t, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_ARRAY_double(name)  _MASON_PARSE_ARRAY_GENERIC(name, double, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_ARRAY_string(name)  _MASON_PARSE_ARRAY_GENERIC(name, string, _MASON_TRANSFORM_STRDUP)
#define MASON_PARSE_ARRAY_bool(name)    _MASON_PARSE_ARRAY_GENERIC(name, bool, _MASON_TRANSFORM_NOOP)
#define MASON_PARSE_ARRAY__Bool(name)   MASON_PARSE_ARRAY_bool(name)

#define MASON_PARSE_OBJECT(type, name)            \
    item = _MASON_BACKEND_GET_FIELD(json, #name); \
    if (_MASON_BACKEND_IS_OBJECT(item)) {         \
        obj->name = type##_from_parsed(item);     \
    }

#define MASON_PARSE_ARRAY_OBJECT(type, name)                         \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                    \
    if (_MASON_BACKEND_IS_ARRAY(item)) {                             \
        obj->name##_count = _MASON_BACKEND_ARRAY_SIZE(item);         \
        obj->name = (type *)calloc(obj->name##_count, sizeof(type)); \
        for (size_t i = 0; i < obj->name##_count; i++) {             \
            MASON_Parsed *elem = _MASON_BACKEND_ARRAY_GET(item, i);  \
            type *parsed = type##_from_parsed(elem);                 \
            if (parsed) {                                            \
                obj->name[i] = *parsed;                              \
                free(parsed);                                        \
            }                                                        \
        }                                                            \
    }

/* Serializer Implementation */

#define _MASON_CREATE_int32_t(val) _MASON_BACKEND_CREATE_int32_t(val)
#define _MASON_CREATE_int64_t(val) _MASON_BACKEND_CREATE_int64_t(val)
#define _MASON_CREATE_double(val)  _MASON_BACKEND_CREATE_double(val)
#define _MASON_CREATE_string(val)  ((val) ? _MASON_BACKEND_CREATE_string(val) : _MASON_BACKEND_CREATE_NULL())
#define _MASON_CREATE_bool(val)    _MASON_BACKEND_CREATE_bool(val)
#define _MASON_CREATE__Bool(val)   _MASON_CREATE_bool(val)

#define _MASON_SERIALIZE_FIELD_GENERIC(name, create_expr) \
    _MASON_BACKEND_OBJECT_ADD(json, #name, create_expr);

#define MASON_SERIALIZE_FIELD_int32_t(name) _MASON_SERIALIZE_FIELD_GENERIC(name, _MASON_CREATE_int32_t(obj->name))
#define MASON_SERIALIZE_FIELD_int64_t(name) _MASON_SERIALIZE_FIELD_GENERIC(name, _MASON_CREATE_int64_t(obj->name))
#define MASON_SERIALIZE_FIELD_double(name)  _MASON_SERIALIZE_FIELD_GENERIC(name, _MASON_CREATE_double(obj->name))
#define MASON_SERIALIZE_FIELD_string(name)  _MASON_SERIALIZE_FIELD_GENERIC(name, _MASON_CREATE_string(obj->name))
#define MASON_SERIALIZE_FIELD_bool(name)    _MASON_SERIALIZE_FIELD_GENERIC(name, _MASON_CREATE_bool(obj->name))
#define MASON_SERIALIZE_FIELD__Bool(name)   MASON_SERIALIZE_FIELD_bool(name)

#define _MASON_SERIALIZE_ARRAY_GENERIC(name, transform)                \
    {                                                                  \
        MASON_Parsed *arr = _MASON_BACKEND_CREATE_ARRAY();             \
        for (size_t i = 0; i < obj->name##_count; i++) {               \
            _MASON_BACKEND_ARRAY_APPEND(arr, transform(obj->name[i])); \
        }                                                              \
        _MASON_BACKEND_OBJECT_ADD(json, #name, arr);                   \
    }

#define MASON_SERIALIZE_ARRAY_int32_t(name) _MASON_SERIALIZE_ARRAY_GENERIC(name, _MASON_CREATE_int32_t)
#define MASON_SERIALIZE_ARRAY_int64_t(name) _MASON_SERIALIZE_ARRAY_GENERIC(name, _MASON_CREATE_int64_t)
#define MASON_SERIALIZE_ARRAY_double(name)  _MASON_SERIALIZE_ARRAY_GENERIC(name, _MASON_CREATE_double)
#define MASON_SERIALIZE_ARRAY_string(name)  _MASON_SERIALIZE_ARRAY_GENERIC(name, _MASON_CREATE_string)
#define MASON_SERIALIZE_ARRAY_bool(name)    _MASON_SERIALIZE_ARRAY_GENERIC(name, _MASON_CREATE_bool)
#define MASON_SERIALIZE_ARRAY__Bool(name)   MASON_SERIALIZE_ARRAY_bool(name)

#define MASON_SERIALIZE_OBJECT(type, name)                  \
    if (obj->name) {                                        \
        MASON_Parsed *nested = type##_to_json(obj->name);   \
        if (nested) {                                       \
            _MASON_BACKEND_OBJECT_ADD(json, #name, nested); \
        }                                                   \
    }

#define MASON_SERIALIZE_ARRAY_OBJECT(type, name)                  \
    {                                                             \
        MASON_Parsed *arr = _MASON_BACKEND_CREATE_ARRAY();        \
        for (size_t i = 0; i < obj->name##_count; i++) {          \
            MASON_Parsed *nested = type##_to_json(&obj->name[i]); \
            if (nested) {                                         \
                _MASON_BACKEND_ARRAY_APPEND(arr, nested);         \
            }                                                     \
        }                                                         \
        _MASON_BACKEND_OBJECT_ADD(json, #name, arr);              \
    }

/* Print Implementation */
#ifdef MASON_PRINT_IMPL

#define MASON_PRINT_INDENT(count)              \
    do {                                       \
        for (int _i = 0; _i < (count); _i++) { \
            putchar(' ');                      \
        }                                      \
    } while (0);

#define _MASON_PRINT_FIELD_GENERIC(name, fmt, val) \
    do {                                           \
        MASON_PRINT_INDENT(_mason_indent);         \
        printf("%s: " fmt "\n", #name, val);       \
    } while (0);

#define MASON_PRINT_FIELD_int32_t(name) _MASON_PRINT_FIELD_GENERIC(name, _MASON_FMT_int32_t, obj->name)
#define MASON_PRINT_FIELD_int64_t(name) _MASON_PRINT_FIELD_GENERIC(name, _MASON_FMT_int64_t, obj->name)
#define MASON_PRINT_FIELD_double(name)  _MASON_PRINT_FIELD_GENERIC(name, _MASON_FMT_double, obj->name)
#define MASON_PRINT_FIELD_string(name)  _MASON_PRINT_FIELD_GENERIC(name, _MASON_FMT_string, obj->name ? obj->name : "null")
#define MASON_PRINT_FIELD_bool(name)    _MASON_PRINT_FIELD_GENERIC(name, _MASON_FMT_bool, obj->name ? "true" : "false")
#define MASON_PRINT_FIELD__Bool(name)   MASON_PRINT_FIELD_bool(name)

#define _MASON_PRINT_ARRAY_GENERIC(name, fmt, val_expr)  \
    do {                                                 \
        MASON_PRINT_INDENT(_mason_indent);               \
        printf("%s[%zu]: [", #name, obj->name##_count);  \
        for (size_t i = 0; i < obj->name##_count; i++) { \
            printf(fmt, val_expr);                       \
            if (i + 1 < obj->name##_count)               \
                printf(", ");                            \
        }                                                \
        printf("]\n");                                   \
    } while (0);

#define MASON_PRINT_ARRAY_int32_t(name) _MASON_PRINT_ARRAY_GENERIC(name, _MASON_FMT_int32_t, obj->name[i])
#define MASON_PRINT_ARRAY_int64_t(name) _MASON_PRINT_ARRAY_GENERIC(name, _MASON_FMT_int64_t, obj->name[i])
#define MASON_PRINT_ARRAY_double(name)  _MASON_PRINT_ARRAY_GENERIC(name, _MASON_FMT_double, obj->name[i])
#define MASON_PRINT_ARRAY_string(name)  _MASON_PRINT_ARRAY_GENERIC(name, _MASON_FMT_string, obj->name[i] ? obj->name[i] : "null")
#define MASON_PRINT_ARRAY_bool(name)    _MASON_PRINT_ARRAY_GENERIC(name, _MASON_FMT_bool, obj->name[i] ? "true" : "false")
#define MASON_PRINT_ARRAY__Bool(name)   MASON_PRINT_ARRAY_bool(name)

#define MASON_PRINT_OBJECT(type, name)                         \
    do {                                                       \
        MASON_PRINT_INDENT(_mason_indent);                     \
        printf("%s: ", #name);                                 \
        if (!obj->name) {                                      \
            printf("null\n");                                  \
        } else {                                               \
            printf("{\n");                                     \
            type##_print_indent(obj->name, _mason_indent + 2); \
            MASON_PRINT_INDENT(_mason_indent);                 \
            printf("}\n");                                     \
        }                                                      \
    } while (0);

#define MASON_PRINT_ARRAY_OBJECT(type, name)                       \
    do {                                                           \
        MASON_PRINT_INDENT(_mason_indent);                         \
        printf("%s[%zu]: [\n", #name, obj->name##_count);          \
        for (size_t i = 0; i < obj->name##_count; i++) {           \
            type##_print_indent(&obj->name[i], _mason_indent + 2); \
        }                                                          \
        MASON_PRINT_INDENT(_mason_indent);                         \
        printf("]\n");                                             \
    } while (0);

#endif // MASON_PRINT_IMPL

/* Memory Management */

#define MASON_FREE_FIELD_int32_t(name)
#define MASON_FREE_FIELD_int64_t(name)
#define MASON_FREE_FIELD_double(name)
#define MASON_FREE_FIELD_bool(name)
#define MASON_FREE_FIELD__Bool(name)

#define MASON_FREE_FIELD_string(name) \
    free(obj->name);

#define _MASON_FREE_ARRAY_SIMPLE(name) free(obj->name);

#define MASON_FREE_ARRAY_int32_t(name) _MASON_FREE_ARRAY_SIMPLE(name)
#define MASON_FREE_ARRAY_int64_t(name) _MASON_FREE_ARRAY_SIMPLE(name)
#define MASON_FREE_ARRAY_double(name)  _MASON_FREE_ARRAY_SIMPLE(name)
#define MASON_FREE_ARRAY_bool(name)    _MASON_FREE_ARRAY_SIMPLE(name)
#define MASON_FREE_ARRAY__Bool(name)   MASON_FREE_ARRAY_bool(name)

#define MASON_FREE_ARRAY_string(name) \
    if (obj->name) {                  \
        type##_free(obj->name);       \
        free(obj->name);              \
    }

#define MASON_FREE_ARRAY_OBJECT(type, name)              \
    if (obj->name) {                                     \
        for (size_t i = 0; i < obj->name##_count; i++) { \
            type##_free_members(&obj->name[i]);          \
        }                                                \
        free(obj->name);                                 \
    }
/* X-Macro Expansion Helpers */

#define MASON_EXPAND_STRUCT_FIELD(type, name)           MASON_FIELD(type, name)
#define MASON_EXPAND_STRUCT_ARRAY(type, name)           MASON_ARRAY(type, name)
#define MASON_EXPAND_STRUCT_ARRAY_MULTI(name)           MASON_ARRAY_MULTI_RAW(name)
#define MASON_EXPAND_STRUCT_OBJECT(type, name)          MASON_OBJECT(type, name)
#define MASON_EXPAND_STRUCT_ARRAY_OBJECT(type, name)    MASON_ARRAY_OBJECT(type, name)

#define MASON_EXPAND_PARSE_FIELD(type, name)            MASON_PARSE_FIELD(type, name)
#define MASON_EXPAND_PARSE_ARRAY(type, name)            _MASON_CONCAT(MASON_PARSE_ARRAY_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_PARSE_ARRAY_MULTI(name)            MASON_PARSE_ARRAY_MULTI(name)
#define MASON_EXPAND_PARSE_OBJECT(type, name)           MASON_PARSE_OBJECT(type, name)
#define MASON_EXPAND_PARSE_ARRAY_OBJECT(type, name)     MASON_PARSE_ARRAY_OBJECT(type, name)

#define MASON_EXPAND_SERIALIZE_FIELD(type, name)        _MASON_CONCAT(MASON_SERIALIZE_FIELD_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_SERIALIZE_ARRAY(type, name)        _MASON_CONCAT(MASON_SERIALIZE_ARRAY_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_SERIALIZE_ARRAY_MULTI(name)        MASON_SERIALIZE_ARRAY_MULTI(name)
#define MASON_EXPAND_SERIALIZE_OBJECT(type, name)       MASON_SERIALIZE_OBJECT(type, name)
#define MASON_EXPAND_SERIALIZE_ARRAY_OBJECT(type, name) MASON_SERIALIZE_ARRAY_OBJECT(type, name)

#define MASON_EXPAND_FREE_FIELD(type, name)             _MASON_CONCAT(MASON_FREE_FIELD_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_FREE_ARRAY(type, name)             _MASON_CONCAT(MASON_FREE_ARRAY_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_FREE_ARRAY_MULTI(name)             MASON_FREE_ARRAY_MULTI(name)
#define MASON_EXPAND_FREE_OBJECT(type, name)            MASON_FREE_OBJECT(type, name)
#define MASON_EXPAND_FREE_ARRAY_OBJECT(type, name)      MASON_FREE_ARRAY_OBJECT(type, name)

#ifdef MASON_PRINT_IMPL
#define MASON_EXPAND_PRINT_FIELD(type, name)        _MASON_CONCAT(MASON_PRINT_FIELD_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_PRINT_ARRAY(type, name)        _MASON_CONCAT(MASON_PRINT_ARRAY_, _MASON_RESOLVE(type))(name)
#define MASON_EXPAND_PRINT_ARRAY_MULTI(name)        MASON_PRINT_ARRAY_MULTI(name)
#define MASON_EXPAND_PRINT_OBJECT(type, name)       MASON_PRINT_OBJECT(type, name)
#define MASON_EXPAND_PRINT_ARRAY_OBJECT(type, name) MASON_PRINT_ARRAY_OBJECT(type, name)
#endif // MASON_PRINT_IMPL

/* Main Implementation Macros */

#define MASON_IMPL_BASE(struct_name, FIELDS)                                                                   \
    struct_name *struct_name##_from_parsed(MASON_Parsed *json) {                                               \
        if (!json)                                                                                             \
            return NULL;                                                                                       \
        struct_name *obj = (struct_name *)calloc(1, sizeof(struct_name));                                      \
        if (!obj)                                                                                              \
            return NULL;                                                                                       \
        MASON_Parsed *item = NULL;                                                                             \
        FIELDS(MASON_EXPAND_PARSE_FIELD, MASON_EXPAND_PARSE_ARRAY, MASON_EXPAND_PARSE_ARRAY_MULTI,             \
               MASON_EXPAND_PARSE_OBJECT, MASON_EXPAND_PARSE_ARRAY_OBJECT)                                     \
        return obj;                                                                                            \
    }                                                                                                          \
                                                                                                               \
    struct_name *struct_name##_from_json(const char *json_str) {                                               \
        MASON_Parsed *parsed = MASON_Parse(json_str);                                                          \
        if (!parsed)                                                                                           \
            return NULL;                                                                                       \
        struct_name *obj = struct_name##_from_parsed(parsed);                                                  \
        _MASON_BACKEND_DELETE(parsed);                                                                         \
        return obj;                                                                                            \
    }                                                                                                          \
                                                                                                               \
    struct_name *struct_name##_from_json_sized(const char *json_str, size_t len) {                             \
        MASON_Parsed *parsed = MASON_ParseSized(json_str, len);                                                \
        if (!parsed)                                                                                           \
            return NULL;                                                                                       \
        struct_name *obj = struct_name##_from_parsed(parsed);                                                  \
        _MASON_BACKEND_DELETE(parsed);                                                                         \
        return obj;                                                                                            \
    }                                                                                                          \
                                                                                                               \
    MASON_Parsed *struct_name##_to_json(struct_name *obj) {                                                    \
        if (!obj)                                                                                              \
            return NULL;                                                                                       \
        MASON_Parsed *json = _MASON_BACKEND_CREATE_OBJECT();                                                   \
        if (!json)                                                                                             \
            return NULL;                                                                                       \
        FIELDS(MASON_EXPAND_SERIALIZE_FIELD, MASON_EXPAND_SERIALIZE_ARRAY, MASON_EXPAND_SERIALIZE_ARRAY_MULTI, \
               MASON_EXPAND_SERIALIZE_OBJECT, MASON_EXPAND_SERIALIZE_ARRAY_OBJECT)                             \
        return json;                                                                                           \
    }                                                                                                          \
                                                                                                               \
    void struct_name##_free_members(struct_name *obj) {                                                        \
        if (!obj)                                                                                              \
            return;                                                                                            \
        FIELDS(MASON_EXPAND_FREE_FIELD, MASON_EXPAND_FREE_ARRAY, MASON_EXPAND_FREE_ARRAY_MULTI,                \
               MASON_EXPAND_FREE_OBJECT, MASON_EXPAND_FREE_ARRAY_OBJECT)                                       \
    }                                                                                                          \
                                                                                                               \
    void struct_name##_free(struct_name *obj) {                                                                \
        if (!obj)                                                                                              \
            return;                                                                                            \
        struct_name##_free_members(obj);                                                                       \
        free(obj);                                                                                             \
    }                                                                                                          \
                                                                                                               \
    string struct_name##_to_string(MASON_Parsed *json) {                                                       \
        if (!json)                                                                                             \
            return NULL;                                                                                       \
        return _MASON_BACKEND_TO_STRING(json);                                                                 \
    }                                                                                                          \
                                                                                                               \
    void struct_name##_string_free(string str) {                                                               \
        if (str)                                                                                               \
            _MASON_BACKEND_STRING_FREE(str);                                                                   \
    }

#ifdef MASON_PRINT_IMPL
#define MASON_IMPL(struct_name, FIELDS)                                                            \
    MASON_IMPL_BASE(struct_name, FIELDS)                                                           \
    void struct_name##_print_indent(struct_name *obj, int indent) {                                \
        if (!obj) {                                                                                \
            MASON_PRINT_INDENT(indent);                                                            \
            printf("%s: null\n", #struct_name);                                                    \
            return;                                                                                \
        }                                                                                          \
        MASON_PRINT_INDENT(indent);                                                                \
        printf("%s {\n", #struct_name);                                                            \
        int _mason_indent = indent + 2;                                                            \
        FIELDS(MASON_EXPAND_PRINT_FIELD, MASON_EXPAND_PRINT_ARRAY, MASON_EXPAND_PRINT_ARRAY_MULTI, \
               MASON_EXPAND_PRINT_OBJECT, MASON_EXPAND_PRINT_ARRAY_OBJECT)                         \
        MASON_PRINT_INDENT(indent);                                                                \
        printf("}\n");                                                                             \
    }                                                                                              \
    void struct_name##_print(struct_name *obj) {                                                   \
        struct_name##_print_indent(obj, 0);                                                        \
    }
#else
#define MASON_IMPL(struct_name, FIELDS)                                                   \
    MASON_IMPL_BASE(struct_name, FIELDS)                                                  \
    void struct_name##_print(struct_name *obj) {                                          \
        (void)obj;                                                                        \
        fprintf(stderr, "%s_print unavailable: define MASON_PRINT_IMPL\n", #struct_name); \
        abort();                                                                          \
    }
#endif

#endif // _MASON_BACKEND

#endif // MASON_H
