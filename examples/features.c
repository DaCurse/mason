#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MASON_PRINT_IMPL
#include "../mason.h"

typedef enum {
    STATUS_OK = 0,
    STATUS_WARN = 1
} Status;

#define MASON_TYPE_ALIAS_Status int32_t

#define ADDRESS_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, street)                                               \
    FIELD(int32_t, zip)

#define PERSON_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                                \
    FIELD(int64_t, id)                                                 \
    FIELD(double, score)                                               \
    FIELD(bool, active)                                                \
    FIELD(Status, status)                                              \
    ARRAY(string, tags)                                                \
    OBJECT(Address, address)                                           \
    ARRAY_OBJECT(Address, history)                                     \
    ARRAY_MULTI(raw)

#define REPORT_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    OBJECT(Person, owner)                                              \
    ARRAY_OBJECT(Person, people)

MASON_STRUCT_DEFINE(Address, ADDRESS_FIELDS)
MASON_STRUCT_DEFINE(Person, PERSON_FIELDS)
MASON_STRUCT_DEFINE(Report, REPORT_FIELDS)

MASON_IMPL(Address, ADDRESS_FIELDS)
MASON_IMPL(Person, PERSON_FIELDS)
MASON_IMPL(Report, REPORT_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

int main(void) {
    size_t json_len = 0;
    char *json_str = mason_read_file_to_string("examples/data/features.json", &json_len);
    if (!json_str) {
        printf("Failed to read examples/data/features.json\n");
        return 1;
    }

    printf("Parsing features example...\n\n");

    Report *report = Report_from_string_sized(json_str, json_len);
    if (!report) {
        printf("Parse failed: %s\n", mason_parse_error());
        free(json_str);
        return 1;
    }
    free(json_str);

    printf("Parsed struct:\n");
    Report_print(report);

    MASON_Parsed round_trip = Report_to_json(report);
    if (!round_trip) {
        printf("Round-trip JSON: null\n");
    } else {
        string json_str = Report_to_string(round_trip);
        if (!json_str) {
            printf("Round-trip JSON: <stringify failed>\n");
        } else {
            printf("Round-trip JSON: %s\n", json_str);
            Report_string_free(json_str);
        }
    }

    if (round_trip)
        mason_delete(round_trip);

    Report_free(report);

    return 0;
}
