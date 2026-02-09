#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MASON_PRINT_IMPL
#define MASON_USE_CJSON
#include "../mason.h"

// https://discord.com/developers/docs/events/gateway-events#activity-object-activity-types
typedef enum {
    ACTIVITY_PLAYING,
    ACTIVITY_STREAMING,
    ACTIVITY_LISTENING,
    ACTIVITY_WATCHING,
    ACTIVITY_CUSTOM,
    ACTIVITY_COMPETING,
} ActivityType;

#define MASON_TYPE_ALIAS_ActivityType int32_t

#define ACTIVITY_BUTTON_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, label)                                                        \
    FIELD(string, url)

#define ACTIVITY_DATA_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                                       \
    FIELD(ActivityType, type)                                                 \
    FIELD(int64_t, created_at)                                                \
    FIELD(string, url)                                                        \
    ARRAY_OBJECT(ActivityButton, buttons)

#define UPDATE_PRESENCE_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(int64_t, since)                                                       \
    FIELD(string, status)                                                       \
    FIELD(bool, afk)                                                            \
    ARRAY_OBJECT(ActivityData, activities)                                      \
    ARRAY_MULTI(test)

MASON_STRUCT_DEFINE(ActivityButton, ACTIVITY_BUTTON_FIELDS)
MASON_STRUCT_DEFINE(ActivityData, ACTIVITY_DATA_FIELDS)
MASON_STRUCT_DEFINE(UpdatePresenceData, UPDATE_PRESENCE_FIELDS)

MASON_IMPL(ActivityButton, ACTIVITY_BUTTON_FIELDS)
MASON_IMPL(ActivityData, ACTIVITY_DATA_FIELDS)
MASON_IMPL(UpdatePresenceData, UPDATE_PRESENCE_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

int main(void) {
    size_t json_len = 0;
    char *json_str = mason_read_file_to_string("examples/data/discord.json", &json_len);
    if (!json_str) {
        printf("Failed to read examples/data/discord.json\n");
        return 1;
    }

    printf("Parsing JSON to UpdatePresenceData\n");
    printf("Input JSON: %s\n\n", json_str);

    UpdatePresenceData *presence = UpdatePresenceData_from_json_sized(json_str, json_len);
    if (!presence) {
        printf("Failed to create UpdatePresenceData from JSON: %s\n", mason_parse_error());
        free(json_str);
        return 1;
    }
    free(json_str);

    printf("Parsed UpdatePresenceData:\n");
    UpdatePresenceData_print(presence);

    UpdatePresenceData_free(presence);

    return 0;
}

