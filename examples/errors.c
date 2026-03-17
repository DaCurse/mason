#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../mason.h"

/* Type alias for testing aliased error messages */

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
} StatusCode;

#define MASON_TYPE_ALIAS_StatusCode int32_t

/* Nested object for OBJECT / ARRAY_OBJECT paths */

#define Address_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, city, REQUIRED)                          \
    FIELD(int32_t, zip, REQUIRED)

MASON_STRUCT_DEFINE(Address, Address_FIELDS)
MASON_IMPL(Address, Address_FIELDS)

/* Struct that uses all categories */

#define User_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name, REQUIRED)                       \
    FIELD(int32_t, age, REQUIRED)                       \
    FIELD(StatusCode, status, REQUIRED)                 \
    ARRAY(int32_t, scores)                              \
    OBJECT(Address, address)                            \
    ARRAY_OBJECT(Address, prev_addresses)

MASON_STRUCT_DEFINE(User, User_FIELDS)
MASON_IMPL(User, User_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

static void try_parse(const char *label, const char *path) {
    size_t len = 0;
    char *json_str = mason_read_file_to_string(path, &len);
    if (!json_str) {
        printf("--- %s ---\n", label);
        printf("failed to read %s\n\n", path);
        return;
    }

    printf("--- %s ---\n", label);

    User *user = User_from_string_sized(json_str, len);
    if (!user) {
        const char *err = mason_error();
        printf("error: %s\n", err ? err : "(null)");
    } else {
        printf("parsed: name=%s age=%d status=%d\n", user->name, user->age, user->status);
    }

    printf("\n");
    free(json_str);
}

#define BASE_DIR "examples/data/errors/"

int main(void) {
    mason_init_default();

    try_parse("valid", BASE_DIR "valid.json");

    /* Parse error */
    try_parse("parse error: malformed JSON", BASE_DIR "malformed.json");

    /* _MASON_PARSE_FIELD errors */
    try_parse("FIELD: wrong primitive type", BASE_DIR "field_wrong_type.json");
    try_parse("FIELD: wrong aliased type", BASE_DIR "field_wrong_alias.json");
    try_parse("FIELD: missing field", BASE_DIR "field_missing.json");

    /* _MASON_PARSE_ARRAY_PRIM errors */
    try_parse("ARRAY: not an array", BASE_DIR "array_not_array.json");
    try_parse("ARRAY: missing field", BASE_DIR "array_missing.json");
    try_parse("ARRAY: wrong element type", BASE_DIR "array_wrong_element.json");

    /* _MASON_PARSE_OBJECT errors */
    try_parse("OBJECT: not an object", BASE_DIR "object_not_object.json");
    try_parse("OBJECT: missing field", BASE_DIR "object_missing.json");
    try_parse("OBJECT: nested field validation fails", BASE_DIR "object_nested_fail.json");

    /* _MASON_PARSE_ARRAY_OBJECT errors */
    try_parse("ARRAY_OBJECT: not an array", BASE_DIR "array_object_not_array.json");
    try_parse("ARRAY_OBJECT: missing field", BASE_DIR "array_object_missing.json");
    try_parse("ARRAY_OBJECT: nested element validation fails", BASE_DIR "array_object_nested_fail.json");

    mason_shutdown();
    return 0;
}
