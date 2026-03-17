#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../mason.h"

#define Profile_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name, REQUIRED)                          \
    FIELD(string, nickname, NULLABLE)                      \
    FIELD(string, email, OPTIONAL)                         \
    FIELD(int32_t, lucky_number, OPTIONAL)

MASON_STRUCT_DEFINE(Profile, Profile_FIELDS)
MASON_IMPL(Profile, Profile_FIELDS)

int main(void) {
    int ret = 0;
    mason_init_default();

    printf("--- parsing ---\n");

    Profile *p0 = Profile_from_string("{\"name\": \"test\"}");
    if (!p0) {
        fprintf(stderr, "Error: %s\n", mason_error());
        ret = 1;
        goto cleanup;
    }
    MASON_Parsed json0 = Profile_to_json(p0);
    char *str0 = Profile_to_string(json0, false);
    printf("Profile 0 : %s\n", str0);

    printf("--- serialization ---\n");

    Profile p1 = {
        .name = "Present",
        .nickname = mason_opt_value("test"),
        .email = mason_opt_value("test@example.com"),
        .lucky_number = mason_opt_value(7),
    };
    MASON_Parsed json1 = Profile_to_json(&p1);
    char *str1 = Profile_to_string(json1, false);
    printf("Profile 1 (all values): %s\n", str1);

    Profile p2 = {
        .name = "Nullable and Missing",
        .nickname = mason_opt_null(),
        .email = mason_opt_missing(),
        .lucky_number = mason_opt_missing(),
    };
    MASON_Parsed json2 = Profile_to_json(&p2);
    char *str2 = Profile_to_string(json2, false);
    printf("Profile 2 (nullable null, optional missing): %s\n", str2);

    Profile p3 = {
        .name = "Optional as null",
        .nickname = mason_opt_value("opt"),
        .email = mason_opt_null(),
    };
    MASON_Parsed json3 = Profile_to_json(&p3);
    char *str3 = Profile_to_string(json3, false);
    printf("Profile 3 (optional null): %s\n", str3);

cleanup:
    mason_shutdown();
    return ret;
}
