#ifndef MASON_H
#define MASON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef MASON_USE_CJSON
#include "backends/cjson.h"
#endif

#ifdef _MASON_BACKEND

static char *_mason_strdup(const char *s) {
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

/* Inline Parsing Helpers */

static inline MASON_Parsed mason_parse(const char *json_str) {
    if (!json_str)
        return NULL;
    return _MASON_BACKEND_PARSE(json_str);
}

static inline MASON_Parsed mason_parse_sized(const char *json_str, size_t len) {
    if (!json_str)
        return NULL;
    return _MASON_BACKEND_PARSE_SIZED(json_str, len);
}

static inline void mason_delete(MASON_Parsed parsed) {
    _MASON_BACKEND_DELETE(parsed);
}

static inline const char *mason_parse_error(void) {
    return _MASON_BACKEND_PARSE_ERROR();
}

/* Field Type Macros */

#define _MASON_FIELD(type, name) type name;
#define _MASON_ARRAY(type, name) \
    type *name;                  \
    size_t name##_count;
#define _MASON_OBJECT(type, name) struct type *name;
#define _MASON_ARRAY_OBJECT(type, name) \
    type *name;                         \
    size_t name##_count;

/* Struct Definition */

#define MASON_STRUCT_DEFINE(struct_name, FIELDS)                                                       \
    typedef struct struct_name {                                                                       \
        FIELDS(_MASON_FIELD, _MASON_ARRAY, _MASON_ARRAY_MULTI_RAW, _MASON_OBJECT, _MASON_ARRAY_OBJECT) \
    } struct_name;                                                                                     \
                                                                                                       \
    struct_name *struct_name##_from_parsed(MASON_Parsed json);                                         \
    struct_name *struct_name##_from_json(const char *json_str);                                        \
    struct_name *struct_name##_from_json_sized(const char *json_str, size_t len);                      \
    MASON_Parsed struct_name##_to_json(struct_name *obj);                                              \
    void struct_name##_free(struct_name *obj);                                                         \
    void struct_name##_free_members(struct_name *obj);                                                 \
    void struct_name##_free_json(MASON_Parsed json);                                                   \
    string struct_name##_to_string(MASON_Parsed json);                                                 \
    void struct_name##_string_free(string str);                                                        \
    void struct_name##_print(struct_name *obj);

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
#define MASON_TYPE_HINT(type)     ((_MASON_TYPE_ALIAS(type))0)

#define MASON_TYPE_ALIAS_int32_t  int32_t
#define MASON_TYPE_ALIAS_int64_t  int64_t
#define MASON_TYPE_ALIAS_double   double
#define MASON_TYPE_ALIAS_string   string
#define MASON_TYPE_ALIAS_bool     bool
#define MASON_TYPE_ALIAS__Bool    bool

/* Inline Type Helpers */

/* Type checkers */
static inline bool mason_is_int32(MASON_Parsed item) { return _MASON_BACKEND_IS_int32_t(item); }
static inline bool mason_is_int64(MASON_Parsed item) { return _MASON_BACKEND_IS_int64_t(item); }
static inline bool mason_is_double(MASON_Parsed item) { return _MASON_BACKEND_IS_double(item); }
static inline bool mason_is_string(MASON_Parsed item) { return _MASON_BACKEND_IS_string(item); }
static inline bool mason_is_bool(MASON_Parsed item) { return _MASON_BACKEND_IS_bool(item); }

/* Non-owning value getters */
static inline int32_t mason_get_int32(MASON_Parsed item) { return _MASON_BACKEND_GET_int32_t(item); }
static inline int64_t mason_get_int64(MASON_Parsed item) { return _MASON_BACKEND_GET_int64_t(item); }
static inline double mason_get_double(MASON_Parsed item) { return _MASON_BACKEND_GET_double(item); }
static inline const char *mason_get_string(MASON_Parsed item) { return _MASON_BACKEND_GET_string(item); }
static inline bool mason_get_bool(MASON_Parsed item) { return _MASON_BACKEND_GET_bool(item); }

/* Owning getters
 * NOTE: strdup for strings, passthrough for primitives
 */
static inline int32_t mason_get_owned_int32(MASON_Parsed item) { return _MASON_BACKEND_GET_int32_t(item); }
static inline int64_t mason_get_owned_int64(MASON_Parsed item) { return _MASON_BACKEND_GET_int64_t(item); }
static inline double mason_get_owned_double(MASON_Parsed item) { return _MASON_BACKEND_GET_double(item); }
static inline char *mason_get_owned_string(MASON_Parsed item) { return _mason_strdup(_MASON_BACKEND_GET_string(item)); }
static inline bool mason_get_owned_bool(MASON_Parsed item) { return _MASON_BACKEND_GET_bool(item); }

/* JSON node creators */
static inline MASON_Parsed mason_create_int32(int32_t v) { return _MASON_BACKEND_CREATE_int32_t(v); }
static inline MASON_Parsed mason_create_int64(int64_t v) { return _MASON_BACKEND_CREATE_int64_t(v); }
static inline MASON_Parsed mason_create_double(double v) { return _MASON_BACKEND_CREATE_double(v); }
static inline MASON_Parsed mason_create_string(const char *v) { return v ? _MASON_BACKEND_CREATE_string(v) : _MASON_BACKEND_CREATE_NULL(); }
static inline MASON_Parsed mason_create_bool(bool v) { return _MASON_BACKEND_CREATE_bool(v); }

/* Field memory free
 * NOTE: noop for non-owning primitives
 */
static inline void mason_free_int32(int32_t v) { (void)v; }
static inline void mason_free_int64(int64_t v) { (void)v; }
static inline void mason_free_double(double v) { (void)v; }
static inline void mason_free_string(char *s) { free(s); }
static inline void mason_free_bool(bool v) { (void)v; }

/* Array memory free */
static inline void mason_free_array_int32(int32_t *arr, size_t count) {
    (void)count;
    free(arr);
}
static inline void mason_free_array_int64(int64_t *arr, size_t count) {
    (void)count;
    free(arr);
}
static inline void mason_free_array_double(double *arr, size_t count) {
    (void)count;
    free(arr);
}
static inline void mason_free_array_bool(bool *arr, size_t count) {
    (void)count;
    free(arr);
}
static inline void mason_free_array_string(char **arr, size_t count) {
    if (arr) {
        for (size_t i = 0; i < count; i++)
            free(arr[i]);
        free(arr);
    }
}

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

#define mason_free_field(value) _Generic((value), \
    int32_t: mason_free_int32,                    \
    int64_t: mason_free_int64,                    \
    double: mason_free_double,                    \
    char *: mason_free_string,                    \
    _Bool: mason_free_bool)(value)

#define mason_free_array(arr, count) _Generic((arr), \
    int32_t *: mason_free_array_int32,               \
    int64_t *: mason_free_array_int64,               \
    double *: mason_free_array_double,               \
    char **: mason_free_array_string,                \
    _Bool *: mason_free_array_bool)(arr, count)

/* Parsing Implementation */

#define _MASON_PARSE_FIELD(type, name)                            \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                 \
    if (mason_is(item, MASON_TYPE_HINT(type))) {                  \
        obj->name = mason_get_owned(item, MASON_TYPE_HINT(type)); \
    }

#define _MASON_PARSE_ARRAY_PRIM(type, name)                                      \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                                \
    if (_MASON_BACKEND_IS_ARRAY(item)) {                                         \
        obj->name##_count = _MASON_BACKEND_ARRAY_SIZE(item);                     \
        obj->name = (type *)calloc(obj->name##_count, sizeof(type));             \
        if (obj->name) {                                                         \
            for (size_t i = 0; i < obj->name##_count; i++) {                     \
                MASON_Parsed elem = _MASON_BACKEND_ARRAY_GET(item, i);           \
                if (mason_is(elem, MASON_TYPE_HINT(type))) {                     \
                    obj->name[i] = mason_get_owned(elem, MASON_TYPE_HINT(type)); \
                }                                                                \
            }                                                                    \
        } else {                                                                 \
            obj->name##_count = 0;                                               \
        }                                                                        \
    }

#define _MASON_PARSE_OBJECT(type, name)           \
    item = _MASON_BACKEND_GET_FIELD(json, #name); \
    if (_MASON_BACKEND_IS_OBJECT(item)) {         \
        obj->name = type##_from_parsed(item);     \
    }

#define _MASON_PARSE_ARRAY_OBJECT(type, name)                          \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                      \
    if (_MASON_BACKEND_IS_ARRAY(item)) {                               \
        obj->name##_count = _MASON_BACKEND_ARRAY_SIZE(item);           \
        obj->name = (type *)calloc(obj->name##_count, sizeof(type));   \
        if (obj->name) {                                               \
            for (size_t i = 0; i < obj->name##_count; i++) {           \
                MASON_Parsed elem = _MASON_BACKEND_ARRAY_GET(item, i); \
                type *parsed = type##_from_parsed(elem);               \
                if (parsed) {                                          \
                    obj->name[i] = *parsed;                            \
                    free(parsed);                                      \
                }                                                      \
            }                                                          \
        } else {                                                       \
            obj->name##_count = 0;                                     \
        }                                                              \
    }

/* Serialization Implementation */

#define _MASON_SERIALIZE_FIELD(type, name) \
    _MASON_BACKEND_OBJECT_ADD(json, #name, mason_create((_MASON_TYPE_ALIAS(type))obj->name));

#define _MASON_SERIALIZE_ARRAY_PRIM(type, name)                                               \
    {                                                                                         \
        MASON_Parsed arr = _MASON_BACKEND_CREATE_ARRAY();                                     \
        for (size_t i = 0; i < obj->name##_count; i++) {                                      \
            _MASON_BACKEND_ARRAY_APPEND(arr,                                                  \
                                        mason_create((_MASON_TYPE_ALIAS(type))obj->name[i])); \
        }                                                                                     \
        _MASON_BACKEND_OBJECT_ADD(json, #name, arr);                                          \
    }

#define _MASON_SERIALIZE_OBJECT(type, name)                 \
    if (obj->name) {                                        \
        MASON_Parsed nested = type##_to_json(obj->name);    \
        if (nested) {                                       \
            _MASON_BACKEND_OBJECT_ADD(json, #name, nested); \
        }                                                   \
    }

#define _MASON_SERIALIZE_ARRAY_OBJECT(type, name)                \
    {                                                            \
        MASON_Parsed arr = _MASON_BACKEND_CREATE_ARRAY();        \
        for (size_t i = 0; i < obj->name##_count; i++) {         \
            MASON_Parsed nested = type##_to_json(&obj->name[i]); \
            if (nested) {                                        \
                _MASON_BACKEND_ARRAY_APPEND(arr, nested);        \
            }                                                    \
        }                                                        \
        _MASON_BACKEND_OBJECT_ADD(json, #name, arr);             \
    }

/* Memory Management */

#define _MASON_FREE_FIELD_DISPATCH(type, name) mason_free_field((_MASON_TYPE_ALIAS(type))obj->name);
#define _MASON_FREE_ARRAY_DISPATCH(type, name) mason_free_array((_MASON_TYPE_ALIAS(type) *)obj->name, obj->name##_count);

#define _MASON_FREE_OBJECT(type, name) \
    if (obj->name) {                   \
        type##_free(obj->name);        \
    }

#define _MASON_FREE_ARRAY_OBJECT(type, name)             \
    if (obj->name) {                                     \
        for (size_t i = 0; i < obj->name##_count; i++) { \
            type##_free_members(&obj->name[i]);          \
        }                                                \
        free(obj->name);                                 \
    }

/* X-Macro Expansion Helpers */

#define _MASON_EXPAND_STRUCT_FIELD(type, name)           _MASON_FIELD(type, name)
#define _MASON_EXPAND_STRUCT_ARRAY(type, name)           _MASON_ARRAY(type, name)
#define _MASON_EXPAND_STRUCT_ARRAY_MULTI(name)           _MASON_ARRAY_MULTI_RAW(name)
#define _MASON_EXPAND_STRUCT_OBJECT(type, name)          _MASON_OBJECT(type, name)
#define _MASON_EXPAND_STRUCT_ARRAY_OBJECT(type, name)    _MASON_ARRAY_OBJECT(type, name)

#define _MASON_EXPAND_PARSE_FIELD(type, name)            _MASON_PARSE_FIELD(type, name)
#define _MASON_EXPAND_PARSE_ARRAY(type, name)            _MASON_PARSE_ARRAY_PRIM(type, name)
#define _MASON_EXPAND_PARSE_ARRAY_MULTI(name)            _MASON_PARSE_ARRAY_MULTI(name)
#define _MASON_EXPAND_PARSE_OBJECT(type, name)           _MASON_PARSE_OBJECT(type, name)
#define _MASON_EXPAND_PARSE_ARRAY_OBJECT(type, name)     _MASON_PARSE_ARRAY_OBJECT(type, name)

#define _MASON_EXPAND_SERIALIZE_FIELD(type, name)        _MASON_SERIALIZE_FIELD(type, name)
#define _MASON_EXPAND_SERIALIZE_ARRAY(type, name)        _MASON_SERIALIZE_ARRAY_PRIM(type, name)
#define _MASON_EXPAND_SERIALIZE_ARRAY_MULTI(name)        _MASON_SERIALIZE_ARRAY_MULTI(name)
#define _MASON_EXPAND_SERIALIZE_OBJECT(type, name)       _MASON_SERIALIZE_OBJECT(type, name)
#define _MASON_EXPAND_SERIALIZE_ARRAY_OBJECT(type, name) _MASON_SERIALIZE_ARRAY_OBJECT(type, name)

#define _MASON_EXPAND_FREE_FIELD(type, name)             _MASON_FREE_FIELD_DISPATCH(type, name)
#define _MASON_EXPAND_FREE_ARRAY(type, name)             _MASON_FREE_ARRAY_DISPATCH(type, name)
#define _MASON_EXPAND_FREE_ARRAY_MULTI(name)             _MASON_FREE_ARRAY_MULTI(name)
#define _MASON_EXPAND_FREE_OBJECT(type, name)            _MASON_FREE_OBJECT(type, name)
#define _MASON_EXPAND_FREE_ARRAY_OBJECT(type, name)      _MASON_FREE_ARRAY_OBJECT(type, name)

/* Multi array support */
#include "mason_multi.h"

/* Print support */
#include "mason_print.h"

/* Main Implementation Macros */

#define _MASON_IMPL_BASE(struct_name, FIELDS)                                                                     \
    struct_name *struct_name##_from_parsed(MASON_Parsed json) {                                                   \
        if (!json)                                                                                                \
            return NULL;                                                                                          \
        struct_name *obj = (struct_name *)calloc(1, sizeof(struct_name));                                         \
        if (!obj)                                                                                                 \
            return NULL;                                                                                          \
        MASON_Parsed item = NULL;                                                                                 \
        FIELDS(_MASON_EXPAND_PARSE_FIELD, _MASON_EXPAND_PARSE_ARRAY, _MASON_EXPAND_PARSE_ARRAY_MULTI,             \
               _MASON_EXPAND_PARSE_OBJECT, _MASON_EXPAND_PARSE_ARRAY_OBJECT)                                      \
        return obj;                                                                                               \
    }                                                                                                             \
                                                                                                                  \
    struct_name *struct_name##_from_json(const char *json_str) {                                                  \
        MASON_Parsed parsed = mason_parse(json_str);                                                              \
        if (!parsed)                                                                                              \
            return NULL;                                                                                          \
        struct_name *obj = struct_name##_from_parsed(parsed);                                                     \
        mason_delete(parsed);                                                                                     \
        return obj;                                                                                               \
    }                                                                                                             \
                                                                                                                  \
    struct_name *struct_name##_from_json_sized(const char *json_str, size_t len) {                                \
        MASON_Parsed parsed = mason_parse_sized(json_str, len);                                                   \
        if (!parsed)                                                                                              \
            return NULL;                                                                                          \
        struct_name *obj = struct_name##_from_parsed(parsed);                                                     \
        mason_delete(parsed);                                                                                     \
        return obj;                                                                                               \
    }                                                                                                             \
                                                                                                                  \
    MASON_Parsed struct_name##_to_json(struct_name *obj) {                                                        \
        if (!obj)                                                                                                 \
            return NULL;                                                                                          \
        MASON_Parsed json = _MASON_BACKEND_CREATE_OBJECT();                                                       \
        if (!json)                                                                                                \
            return NULL;                                                                                          \
        FIELDS(_MASON_EXPAND_SERIALIZE_FIELD, _MASON_EXPAND_SERIALIZE_ARRAY, _MASON_EXPAND_SERIALIZE_ARRAY_MULTI, \
               _MASON_EXPAND_SERIALIZE_OBJECT, _MASON_EXPAND_SERIALIZE_ARRAY_OBJECT)                              \
        return json;                                                                                              \
    }                                                                                                             \
                                                                                                                  \
    void struct_name##_free_members(struct_name *obj) {                                                           \
        if (!obj)                                                                                                 \
            return;                                                                                               \
        FIELDS(_MASON_EXPAND_FREE_FIELD, _MASON_EXPAND_FREE_ARRAY, _MASON_EXPAND_FREE_ARRAY_MULTI,                \
               _MASON_EXPAND_FREE_OBJECT, _MASON_EXPAND_FREE_ARRAY_OBJECT)                                        \
    }                                                                                                             \
                                                                                                                  \
    void struct_name##_free(struct_name *obj) {                                                                   \
        if (!obj)                                                                                                 \
            return;                                                                                               \
        struct_name##_free_members(obj);                                                                          \
        free(obj);                                                                                                \
    }                                                                                                             \
                                                                                                                  \
    string struct_name##_to_string(MASON_Parsed json) {                                                           \
        if (!json)                                                                                                \
            return NULL;                                                                                          \
        return _MASON_BACKEND_TO_STRING(json);                                                                    \
    }                                                                                                             \
                                                                                                                  \
    void struct_name##_string_free(string str) {                                                                  \
        if (str)                                                                                                  \
            _MASON_BACKEND_STRING_FREE(str);                                                                      \
    }

#define MASON_IMPL(struct_name, FIELDS)   \
    _MASON_IMPL_BASE(struct_name, FIELDS) \
    _MASON_IMPL_PRINT(struct_name, FIELDS)

#endif // _MASON_BACKEND

#endif // MASON_H
