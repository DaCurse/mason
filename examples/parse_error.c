#include <stdio.h>

#include "../mason.h"

#define Example_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)

MASON_STRUCT_DEFINE(Example, Example_FIELDS)
MASON_IMPL(Example, Example_FIELDS)

int main(void) {
    mason_init_default();

    const char *bad_json = "{\"name\": \"oops\",}";
    Example *ex = Example_from_string(bad_json);
    if (!ex) {
        const char *err = mason_parse_error();
        printf("parse failed: %s\n", err ? err : "(null)");
    } else {
        printf("unexpected success: %s\n", ex->name);
    }

    mason_shutdown();
    return 0;
}
