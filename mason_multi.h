#ifdef _MASON_BACKEND

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
} Mason_RawValueType;

typedef struct mason_value {
    Mason_RawValueType type;
    union {
        int32_t i32;
        int64_t i64;
        double d;
        char *s;
        bool b;
        MASON_Parsed *ast;
    } value;
} Mason_RawValue;

/* Helper constructors */

static inline Mason_RawValue mason_rawvalue_int32_t(int32_t val) {
    Mason_RawValue v = {MASON_VALUE_INT32, {.i32 = val}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_int64_t(int64_t val) {
    Mason_RawValue v = {MASON_VALUE_INT64, {.i64 = val}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_double(double val) {
    Mason_RawValue v = {MASON_VALUE_DOUBLE, {.d = val}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_string(const char *val) {
    Mason_RawValue v = {MASON_VALUE_STRING, {.s = val ? mason_strdup(val) : NULL}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_bool(bool val) {
    Mason_RawValue v = {MASON_VALUE_BOOL, {.b = val}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_null(void) {
    Mason_RawValue v = {MASON_VALUE_NULL, {.i32 = 0}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_object(MASON_Parsed *ast) {
    Mason_RawValue v = {MASON_VALUE_OBJECT, {.ast = ast}};
    return v;
}

static inline Mason_RawValue mason_rawvalue_array(MASON_Parsed *arr) {
    Mason_RawValue v = {MASON_VALUE_ARRAY, {.ast = arr}};
    return v;
}

static inline void mason_rawvalue_free(Mason_RawValue *val) {
    if (!val)
        return;
    switch (val->type) {
    case MASON_VALUE_STRING:
        free(val->value.s);
        break;
    case MASON_VALUE_OBJECT:
        // Objects are references to parent JSON, not owned
    case MASON_VALUE_ARRAY:
        // Arrays are references to parent JSON, not owned
    default:
        break;
    }
}

/* Field Type Macro */

#define MASON_ARRAY_MULTI_RAW(name) \
    Mason_RawValue *name;           \
    size_t name##_count;

/* Parser */

#define MASON_PARSE_ARRAY_MULTI(name)                                                    \
    item = _MASON_BACKEND_GET_FIELD(json, #name);                                        \
    if (_MASON_BACKEND_IS_ARRAY(item)) {                                                 \
        obj->name##_count = _MASON_BACKEND_ARRAY_SIZE(item);                             \
        obj->name = (Mason_RawValue *)calloc(obj->name##_count, sizeof(Mason_RawValue)); \
        for (size_t i = 0; i < obj->name##_count; i++) {                                 \
            MASON_Parsed *elem = _MASON_BACKEND_ARRAY_GET(item, i);                      \
            if (_MASON_BACKEND_IS_int64_t(elem)) {                                       \
                double d = _MASON_BACKEND_GET_double(elem);                              \
                if (d == (int64_t)d) {                                                   \
                    if (d >= INT32_MIN && d <= INT32_MAX) {                              \
                        obj->name[i] = mason_rawvalue_int32_t((int32_t)d);               \
                    } else {                                                             \
                        obj->name[i] = mason_rawvalue_int64_t((int64_t)d);               \
                    }                                                                    \
                } else {                                                                 \
                    obj->name[i] = mason_rawvalue_double(d);                             \
                }                                                                        \
            } else if (_MASON_BACKEND_IS_string(elem)) {                                 \
                obj->name[i] = mason_rawvalue_string(_MASON_BACKEND_GET_string(elem));   \
            } else if (_MASON_BACKEND_IS_bool(elem)) {                                   \
                obj->name[i] = mason_rawvalue_bool(_MASON_BACKEND_GET_bool(elem));       \
            } else if (_MASON_BACKEND_IS_NULL(elem)) {                                   \
                obj->name[i] = mason_rawvalue_null();                                    \
            } else if (_MASON_BACKEND_IS_ARRAY(elem)) {                                  \
                obj->name[i] = mason_rawvalue_array(elem);                               \
            } else if (_MASON_BACKEND_IS_OBJECT(elem)) {                                 \
                obj->name[i] = mason_rawvalue_object(elem);                              \
            }                                                                            \
        }                                                                                \
    }

/* Serializer */

#define MASON_SERIALIZE_ARRAY_MULTI(name)                                                                 \
    {                                                                                                     \
        MASON_Parsed *arr = _MASON_BACKEND_CREATE_ARRAY();                                                \
        for (size_t i = 0; i < obj->name##_count; i++) {                                                  \
            switch (obj->name[i].type) {                                                                  \
            case MASON_VALUE_INT32:                                                                       \
                _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_int32_t(obj->name[i].value.i32));  \
                break;                                                                                    \
            case MASON_VALUE_INT64:                                                                       \
                _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_int64_t(obj->name[i].value.i64));  \
                break;                                                                                    \
            case MASON_VALUE_DOUBLE:                                                                      \
                _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_double(obj->name[i].value.d));     \
                break;                                                                                    \
            case MASON_VALUE_STRING:                                                                      \
                if (obj->name[i].value.s) {                                                               \
                    _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_string(obj->name[i].value.s)); \
                } else {                                                                                  \
                    _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_NULL());                       \
                }                                                                                         \
                break;                                                                                    \
            case MASON_VALUE_BOOL:                                                                        \
                _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_bool(obj->name[i].value.b));       \
                break;                                                                                    \
            case MASON_VALUE_NULL:                                                                        \
                _MASON_BACKEND_ARRAY_APPEND(arr, _MASON_BACKEND_CREATE_NULL());                           \
                break;                                                                                    \
            case MASON_VALUE_ARRAY:                                                                       \
                if (obj->name[i].value.ast) {                                                             \
                    _MASON_BACKEND_ARRAY_APPEND(arr, obj->name[i].value.ast);                             \
                }                                                                                         \
                break;                                                                                    \
            case MASON_VALUE_OBJECT:                                                                      \
                if (obj->name[i].value.ast) {                                                             \
                    _MASON_BACKEND_ARRAY_APPEND(arr, obj->name[i].value.ast);                             \
                }                                                                                         \
                break;                                                                                    \
            default:                                                                                      \
                break;                                                                                    \
            }                                                                                             \
        }                                                                                                 \
        _MASON_BACKEND_OBJECT_ADD(json, #name, arr);                                                      \
    }

/* Print */
#ifdef MASON_PRINT_IMPL

#define MASON_PRINT_ARRAY_MULTI(name)                                                            \
    do {                                                                                         \
        MASON_PRINT_INDENT(_mason_indent);                                                       \
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

#define MASON_FREE_ARRAY_MULTI(name)                     \
    if (obj->name) {                                     \
        for (size_t i = 0; i < obj->name##_count; i++) { \
            mason_rawvalue_free(&obj->name[i]);          \
        }                                                \
        free(obj->name);                                 \
    }

#endif // _MASON_BACKEND
