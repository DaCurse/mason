#ifndef MASON_MULTI_H
#define MASON_MULTI_H

/* Multi-Value Array Types */

typedef enum {
    MASON_VALUE_NULL,
    MASON_VALUE_INT32,
    MASON_VALUE_INT64,
    MASON_VALUE_DOUBLE,
    MASON_VALUE_STRING,
    MASON_VALUE_BOOL,
    MASON_VALUE_OBJECT,
    MASON_VALUE_ARRAY
} MASON_RawValueType;

typedef struct {
    MASON_RawValueType type;
    union {
        int32_t i32;
        int64_t i64;
        double d;
        char *s;
        bool b;
        MASON_Parsed ast;
    } value;
} MASON_RawValue;

/* Helper constructors */

static inline MASON_RawValue mason_rawvalue_int32_t(int32_t val) {
    MASON_RawValue v = {MASON_VALUE_INT32, {.i32 = val}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_int64_t(int64_t val) {
    MASON_RawValue v = {MASON_VALUE_INT64, {.i64 = val}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_double(double val) {
    MASON_RawValue v = {MASON_VALUE_DOUBLE, {.d = val}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_string(const char *val) {
    MASON_RawValue v = {MASON_VALUE_STRING, {.s = val ? _mason_strdup(val) : NULL}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_bool(bool val) {
    MASON_RawValue v = {MASON_VALUE_BOOL, {.b = val}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_null(void) {
    MASON_RawValue v = {MASON_VALUE_NULL, {.i32 = 0}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_object(MASON_Parsed ast) {
    MASON_RawValue v = {MASON_VALUE_OBJECT, {.ast = ast}};
    return v;
}

static inline MASON_RawValue mason_rawvalue_array(MASON_Parsed arr) {
    MASON_RawValue v = {MASON_VALUE_ARRAY, {.ast = arr}};
    return v;
}

static inline void mason_rawvalue_free(MASON_RawValue *val) {
    if (!val)
        return;
    switch (val->type) {
    case MASON_VALUE_STRING:
        free(val->value.s);
        break;
    case MASON_VALUE_OBJECT:
        mason_delete(val->value.ast);
        break;
    case MASON_VALUE_ARRAY:
        mason_delete(val->value.ast);
        break;
    default:
        break;
    }
}

/* Field Type Macro */

#define _MASON_ARRAY_MULTI_RAW(name) \
    MASON_RawValue *name;            \
    size_t name##_count;

/* Parser */

#define _MASON_PARSE_ARRAY_MULTI(name)                                                   \
    item = cJSON_GetObjectItemCaseSensitive(json, #name);                                \
    if (cJSON_IsArray(item)) {                                                           \
        obj->name##_count = (size_t)cJSON_GetArraySize(item);                            \
        obj->name = (MASON_RawValue *)calloc(obj->name##_count, sizeof(MASON_RawValue)); \
        if (obj->name) {                                                                 \
            for (size_t i = 0; i < obj->name##_count; i++) {                             \
                MASON_Parsed elem = cJSON_GetArrayItem(item, (int)i);                    \
                if (cJSON_IsNumber(elem)) {                                              \
                    double d = elem->valuedouble;                                        \
                    if (d == (int64_t)d) {                                               \
                        if (d >= INT32_MIN && d <= INT32_MAX) {                          \
                            obj->name[i] = mason_rawvalue_int32_t((int32_t)d);           \
                        } else {                                                         \
                            obj->name[i] = mason_rawvalue_int64_t((int64_t)d);           \
                        }                                                                \
                    } else {                                                             \
                        obj->name[i] = mason_rawvalue_double(d);                         \
                    }                                                                    \
                } else if (cJSON_IsString(elem) && elem->valuestring) {                  \
                    obj->name[i] = mason_rawvalue_string(elem->valuestring);             \
                } else if (cJSON_IsBool(elem)) {                                         \
                    obj->name[i] = mason_rawvalue_bool(cJSON_IsTrue(elem));              \
                } else if (cJSON_IsNull(elem)) {                                         \
                    obj->name[i] = mason_rawvalue_null();                                \
                } else if (cJSON_IsArray(elem)) {                                        \
                    MASON_Parsed dup = cJSON_Duplicate(elem, 1);                         \
                    if (dup)                                                             \
                        obj->name[i] = mason_rawvalue_array(dup);                        \
                } else if (cJSON_IsObject(elem)) {                                       \
                    MASON_Parsed dup = cJSON_Duplicate(elem, 1);                         \
                    if (dup)                                                             \
                        obj->name[i] = mason_rawvalue_object(dup);                       \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            obj->name##_count = 0;                                                       \
        }                                                                                \
    }

/* Serializer */

#define _MASON_SERIALIZE_ARRAY_MULTI(name)                                                     \
    {                                                                                          \
        MASON_Parsed arr = cJSON_CreateArray();                                                \
        for (size_t i = 0; i < obj->name##_count; i++) {                                       \
            switch (obj->name[i].type) {                                                       \
            case MASON_VALUE_INT32:                                                            \
                cJSON_AddItemToArray(arr, cJSON_CreateNumber(obj->name[i].value.i32));         \
                break;                                                                         \
            case MASON_VALUE_INT64:                                                            \
                cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)obj->name[i].value.i64)); \
                break;                                                                         \
            case MASON_VALUE_DOUBLE:                                                           \
                cJSON_AddItemToArray(arr, cJSON_CreateNumber(obj->name[i].value.d));           \
                break;                                                                         \
            case MASON_VALUE_STRING:                                                           \
                if (obj->name[i].value.s) {                                                    \
                    cJSON_AddItemToArray(arr, cJSON_CreateString(obj->name[i].value.s));       \
                } else {                                                                       \
                    cJSON_AddItemToArray(arr, cJSON_CreateNull());                             \
                }                                                                              \
                break;                                                                         \
            case MASON_VALUE_BOOL:                                                             \
                cJSON_AddItemToArray(arr, cJSON_CreateBool(obj->name[i].value.b));             \
                break;                                                                         \
            case MASON_VALUE_NULL:                                                             \
                cJSON_AddItemToArray(arr, cJSON_CreateNull());                                 \
                break;                                                                         \
            case MASON_VALUE_ARRAY:                                                            \
                if (obj->name[i].value.ast) {                                                  \
                    MASON_Parsed dup = cJSON_Duplicate(obj->name[i].value.ast, 1);             \
                    if (dup)                                                                   \
                        cJSON_AddItemToArray(arr, dup);                                        \
                }                                                                              \
                break;                                                                         \
            case MASON_VALUE_OBJECT:                                                           \
                if (obj->name[i].value.ast) {                                                  \
                    MASON_Parsed dup = cJSON_Duplicate(obj->name[i].value.ast, 1);             \
                    if (dup)                                                                   \
                        cJSON_AddItemToArray(arr, dup);                                        \
                }                                                                              \
                break;                                                                         \
            default:                                                                           \
                break;                                                                         \
            }                                                                                  \
        }                                                                                      \
        cJSON_AddItemToObject(json, #name, arr);                                               \
    }

/* Print */
#ifdef MASON_PRINT_IMPL

#define _MASON_PRINT_ARRAY_MULTI(name)                                                           \
    do {                                                                                         \
        _MASON_PRINT_INDENT(_mason_indent);                                                      \
        printf("%s[%zu]: [", #name, obj->name##_count);                                          \
        for (size_t i = 0; i < obj->name##_count; i++) {                                         \
            switch (obj->name[i].type) {                                                         \
            case MASON_VALUE_INT32:                                                              \
                printf(_MASON_FMT_int32_t, obj->name[i].value.i32);                              \
                break;                                                                           \
            case MASON_VALUE_INT64:                                                              \
                printf(_MASON_FMT_int64_t, obj->name[i].value.i64);                              \
                break;                                                                           \
            case MASON_VALUE_DOUBLE:                                                             \
                printf(_MASON_FMT_double, obj->name[i].value.d);                                 \
                break;                                                                           \
            case MASON_VALUE_STRING:                                                             \
                printf(_MASON_FMT_string, obj->name[i].value.s ? obj->name[i].value.s : "null"); \
                break;                                                                           \
            case MASON_VALUE_BOOL:                                                               \
                printf(_MASON_FMT_bool, obj->name[i].value.b ? "true" : "false");                \
                break;                                                                           \
            case MASON_VALUE_NULL:                                                               \
                printf("null");                                                                  \
                break;                                                                           \
            case MASON_VALUE_ARRAY:                                                              \
            case MASON_VALUE_OBJECT:                                                             \
                printf("AST@%p", (void *)obj->name[i].value.ast);                                \
                break;                                                                           \
            default:                                                                             \
                printf("unknown");                                                               \
                break;                                                                           \
            }                                                                                    \
            if (i + 1 < obj->name##_count)                                                       \
                printf(", ");                                                                    \
        }                                                                                        \
        printf("]\n");                                                                           \
    } while (0);

#endif // MASON_PRINT_IMPL

/* Memory Management */

#define _MASON_FREE_ARRAY_MULTI(name)                    \
    if (obj->name) {                                     \
        for (size_t i = 0; i < obj->name##_count; i++) { \
            mason_rawvalue_free(&obj->name[i]);          \
        }                                                \
        free(obj->name);                                 \
    }

#endif // MASON_MULTI_H
