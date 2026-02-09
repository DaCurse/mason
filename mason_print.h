#ifdef _MASON_BACKEND

/* Print Implementation */
#ifdef MASON_PRINT_IMPL

#include <inttypes.h>
#include <stdio.h>

/* Format Strings */

#define _MASON_FMT_int32_t "%" PRId32
#define _MASON_FMT_int64_t "%" PRId64
#define _MASON_FMT_double  "%f"
#define _MASON_FMT_string  "\"%s\""
#define _MASON_FMT_bool    "%s"

/* Indent helper */

#define _MASON_PRINT_INDENT(count)             \
    do {                                       \
        for (int _i = 0; _i < (count); _i++) { \
            putchar(' ');                      \
        }                                      \
    } while (0);

/* Print inline helpers */

static inline void _mason_print_indent(int count) {
    for (int i = 0; i < count; i++)
        putchar(' ');
}

static inline void mason_print_int32(const char *name, int32_t v, int indent) {
    _mason_print_indent(indent);
    printf("%s: %" PRId32 "\n", name, v);
}
static inline void mason_print_int64(const char *name, int64_t v, int indent) {
    _mason_print_indent(indent);
    printf("%s: %" PRId64 "\n", name, v);
}
static inline void mason_print_double(const char *name, double v, int indent) {
    _mason_print_indent(indent);
    printf("%s: %f\n", name, v);
}
static inline void mason_print_string(const char *name, const char *v, int indent) {
    _mason_print_indent(indent);
    printf("%s: \"%s\"\n", name, v ? v : "null");
}
static inline void mason_print_bool(const char *name, bool v, int indent) {
    _mason_print_indent(indent);
    printf("%s: %s\n", name, v ? "true" : "false");
}

#define mason_print_field(name_str, value, indent) _Generic((value), \
    int32_t: mason_print_int32,                                      \
    int64_t: mason_print_int64,                                      \
    double: mason_print_double,                                      \
    char *: mason_print_string,                                      \
    const char *: mason_print_string,                                \
    _Bool: mason_print_bool)(name_str, value, indent)

/* Array element printers */
static inline void mason_print_array_elem_int32(int32_t v) { printf("%" PRId32, v); }
static inline void mason_print_array_elem_int64(int64_t v) { printf("%" PRId64, v); }
static inline void mason_print_array_elem_double(double v) { printf("%f", v); }
static inline void mason_print_array_elem_string(const char *v) { printf("\"%s\"", v ? v : "null"); }
static inline void mason_print_array_elem_bool(bool v) { printf("%s", v ? "true" : "false"); }

#define mason_print_array_elem(value) _Generic((value), \
    int32_t: mason_print_array_elem_int32,              \
    int64_t: mason_print_array_elem_int64,              \
    double: mason_print_array_elem_double,              \
    char *: mason_print_array_elem_string,              \
    const char *: mason_print_array_elem_string,        \
    _Bool: mason_print_array_elem_bool)(value)

#define _MASON_PRINT_FIELD_DISPATCH(type, name) \
    mason_print_field(#name, (_MASON_TYPE_ALIAS(type))obj->name, _mason_indent);

#define _MASON_PRINT_ARRAY_DISPATCH(type, name)                            \
    do {                                                                   \
        _mason_print_indent(_mason_indent);                                \
        printf("%s[%zu]: [", #name, obj->name##_count);                    \
        for (size_t i = 0; i < obj->name##_count; i++) {                   \
            mason_print_array_elem((_MASON_TYPE_ALIAS(type))obj->name[i]); \
            if (i + 1 < obj->name##_count)                                 \
                printf(", ");                                              \
        }                                                                  \
        printf("]\n");                                                     \
    } while (0);

#define _MASON_PRINT_OBJECT(type, name)                        \
    do {                                                       \
        _MASON_PRINT_INDENT(_mason_indent);                    \
        printf("%s: ", #name);                                 \
        if (!obj->name) {                                      \
            printf("null\n");                                  \
        } else {                                               \
            printf("{\n");                                     \
            type##_print_indent(obj->name, _mason_indent + 2); \
            _MASON_PRINT_INDENT(_mason_indent);                \
            printf("}\n");                                     \
        }                                                      \
    } while (0);

#define _MASON_PRINT_ARRAY_OBJECT(type, name)                      \
    do {                                                           \
        _MASON_PRINT_INDENT(_mason_indent);                        \
        printf("%s[%zu]: [\n", #name, obj->name##_count);          \
        for (size_t i = 0; i < obj->name##_count; i++) {           \
            type##_print_indent(&obj->name[i], _mason_indent + 2); \
        }                                                          \
        _MASON_PRINT_INDENT(_mason_indent);                        \
        printf("]\n");                                             \
    } while (0);

/* X-Macro Expansion Helpers for Print */

#define _MASON_EXPAND_PRINT_FIELD(type, name)        _MASON_PRINT_FIELD_DISPATCH(type, name)
#define _MASON_EXPAND_PRINT_ARRAY(type, name)        _MASON_PRINT_ARRAY_DISPATCH(type, name)
#define _MASON_EXPAND_PRINT_ARRAY_MULTI(name)        _MASON_PRINT_ARRAY_MULTI(name)
#define _MASON_EXPAND_PRINT_OBJECT(type, name)       _MASON_PRINT_OBJECT(type, name)
#define _MASON_EXPAND_PRINT_ARRAY_OBJECT(type, name) _MASON_PRINT_ARRAY_OBJECT(type, name)

/* Partial print impl */
#define _MASON_IMPL_PRINT(struct_name, FIELDS)                                                        \
    void struct_name##_print_indent(struct_name *obj, int indent) {                                   \
        if (!obj) {                                                                                   \
            _MASON_PRINT_INDENT(indent);                                                              \
            printf("%s: null\n", #struct_name);                                                       \
            return;                                                                                   \
        }                                                                                             \
        _MASON_PRINT_INDENT(indent);                                                                  \
        printf("%s {\n", #struct_name);                                                               \
        int _mason_indent = indent + 2;                                                               \
        FIELDS(_MASON_EXPAND_PRINT_FIELD, _MASON_EXPAND_PRINT_ARRAY, _MASON_EXPAND_PRINT_ARRAY_MULTI, \
               _MASON_EXPAND_PRINT_OBJECT, _MASON_EXPAND_PRINT_ARRAY_OBJECT)                          \
        _MASON_PRINT_INDENT(indent);                                                                  \
        printf("}\n");                                                                                \
    }                                                                                                 \
    void struct_name##_print(struct_name *obj) {                                                      \
        struct_name##_print_indent(obj, 0);                                                           \
    }

#else // !MASON_PRINT_IMPL

#define _MASON_IMPL_PRINT(struct_name, FIELDS)                                            \
    void struct_name##_print(struct_name *obj) {                                          \
        (void)obj;                                                                        \
        fprintf(stderr, "%s_print unavailable: define MASON_PRINT_IMPL\n", #struct_name); \
        abort();                                                                          \
    }

#endif // MASON_PRINT_IMPL

#endif // _MASON_BACKEND

