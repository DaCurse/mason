#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MASON_PRINT_IMPL
#define MASON_USE_CJSON
#include "../mason.h"

// https://discord.com/developers/docs/topics/opcodes-and-status-codes#gateway-gateway-opcodes
typedef enum {
    SEND_OPCODE_HEARTBEAT = 1,
    SEND_OPCODE_IDENTIFY = 2,
    SEND_OPCODE_RESUME = 6,
} GatewayOpcodeSend;

#define MASON_TYPE_ALIAS_GatewayOpcodeSend int32_t

// https://discord.com/developers/docs/events/gateway-events#identify-identify-structure
#define IDENTIFY_PROPERTIES_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, os)                                                               \
    FIELD(string, browser)                                                          \
    FIELD(string, device)

// https://discord.com/developers/docs/topics/gateway-events#activity-object
#define IDENTIFY_ACTIVITY_BUTTON_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, label)                                                                 \
    FIELD(string, url)

#define IDENTIFY_ACTIVITY_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                                           \
    FIELD(int32_t, type)                                                          \
    FIELD(int64_t, created_at)                                                    \
    FIELD(string, url)                                                            \
    ARRAY_OBJECT(IdentifyActivityButton, buttons)

// https://discord.com/developers/docs/events/gateway-events#presence-update
#define IDENTIFY_PRESENCE_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(int64_t, since)                                                         \
    FIELD(string, status)                                                         \
    FIELD(bool, afk)                                                              \
    ARRAY_OBJECT(IdentifyActivity, activities)

#define IDENTIFY_EVENT_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, token)                                                       \
    OBJECT(IdentifyProperties, properties)                                     \
    OBJECT(IdentifyPresence, presence)                                         \
    FIELD(int32_t, intents)

// https://discord.com/developers/docs/events/gateway-events#payload-structure
#define GATEWAY_EVENT_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(GatewayOpcodeSend, op)                                              \
    OBJECT(IdentifyEventData, d)

MASON_STRUCT_DEFINE(IdentifyProperties, IDENTIFY_PROPERTIES_FIELDS)
MASON_STRUCT_DEFINE(IdentifyActivityButton, IDENTIFY_ACTIVITY_BUTTON_FIELDS)
MASON_STRUCT_DEFINE(IdentifyActivity, IDENTIFY_ACTIVITY_FIELDS)
MASON_STRUCT_DEFINE(IdentifyPresence, IDENTIFY_PRESENCE_FIELDS)
MASON_STRUCT_DEFINE(IdentifyEventData, IDENTIFY_EVENT_FIELDS)
MASON_STRUCT_DEFINE(GatewayEventPayload, GATEWAY_EVENT_FIELDS)

MASON_IMPL(IdentifyProperties, IDENTIFY_PROPERTIES_FIELDS)
MASON_IMPL(IdentifyActivityButton, IDENTIFY_ACTIVITY_BUTTON_FIELDS)
MASON_IMPL(IdentifyActivity, IDENTIFY_ACTIVITY_FIELDS)
MASON_IMPL(IdentifyPresence, IDENTIFY_PRESENCE_FIELDS)
MASON_IMPL(IdentifyEventData, IDENTIFY_EVENT_FIELDS)
MASON_IMPL(GatewayEventPayload, GATEWAY_EVENT_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

int main(void) {
    size_t json_len = 0;
    char *json_str = mason_read_file_to_string("examples/data/discord.json", &json_len);
    if (!json_str) {
        printf("Failed to read examples/data/discord.json\n");
        return 1;
    }

    printf("Parsing JSON to GatewayEventPayload\n");
    printf("Input JSON: %s\n\n", json_str);

    GatewayEventPayload *payload = GatewayEventPayload_from_json_sized(json_str, json_len);
    if (!payload) {
        printf("Failed to create GatewayEventPayload from JSON: %s\n", mason_parse_error());
        free(json_str);
        return 1;
    }
    free(json_str);

    printf("Parsed GatewayEventPayload:\n");
    GatewayEventPayload_print(payload);

    GatewayEventPayload_free(payload);

    return 0;
}

