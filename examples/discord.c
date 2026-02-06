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

#define MASON_BASE_TYPE_ActivityType int32_t

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

int main(void) {
    printf("Parsing JSON to UpdatePresenceData\n");
    // clang-format off
    const char *json_str =
        "{"
            "\"since\": 17000000123,"
            "\"status\": \"online\","
            "\"afk\": false,"
            "\"activities\": ["
                "{"
                    "\"name\": \"Playing Mason\","
                    "\"type\": 0,"
                    "\"created_at\": 4320456,"
                    "\"url\": null,"
                    "\"buttons\": ["
                        "{\"label\": \"Join\", \"url\": \"https://example.com\"},"
                        "{\"label\": \"Watch\", \"url\": \"https://example.com/live\"}"
                    "]"
                "}"
            "],"
            "\"test\":[1,2.5,\"a\", [], {}]"
        "}";
    // clang-format on
    printf("Input JSON: %s\n\n", json_str);

    UpdatePresenceData *presence = UpdatePresenceData_from_json(json_str);
    if (!presence) {
        printf("Failed to create UpdatePresenceData from JSON: %s\n", MASON_ParseError());
        return 1;
    }

    printf("Parsed UpdatePresenceData:\n");
    UpdatePresenceData_print(presence);

    UpdatePresenceData_free(presence);

    return 0;
}

