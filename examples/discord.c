#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../mason.h"

// https://discord.com/developers/docs/topics/opcodes-and-status-codes#gateway-gateway-opcodes
typedef enum {
    SEND_OPCODE_HEARTBEAT = 1,
    SEND_OPCODE_IDENTIFY = 2,
    SEND_OPCODE_RESUME = 6,
} GatewayOpcodeSend;

#define MASON_TYPE_ALIAS_GatewayOpcodeSend int32_t

// https://discord.com/developers/docs/events/gateway-events#identify-identify-structure
#define IdentifyProperties_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, os)                                                 \
    FIELD(string, browser)                                            \
    FIELD(string, device)

// https://discord.com/developers/docs/topics/gateway-events#activity-object
#define IdentifyActivityButton_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, label)                                                  \
    FIELD(string, url)

#define IdentifyActivity_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                             \
    FIELD(int32_t, type)                                            \
    FIELD(int64_t, created_at)                                      \
    FIELD(string, url)                                              \
    ARRAY_OBJECT(IdentifyActivityButton, buttons)

// https://discord.com/developers/docs/events/gateway-events#presence-update
#define IdentifyPresence_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(int64_t, since)                                           \
    FIELD(string, status)                                           \
    FIELD(bool, afk)                                                \
    ARRAY_OBJECT(IdentifyActivity, activities)

#define IdentifyEvent_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, token)                                         \
    OBJECT(IdentifyProperties, properties)                       \
    OBJECT(IdentifyPresence, presence)                           \
    FIELD(int32_t, intents)

// https://discord.com/developers/docs/events/gateway-events#payload-structure
#define GatewayEventPayload_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(GatewayOpcodeSend, op)                                       \
    OBJECT(IdentifyEventData, d)

MASON_STRUCT_DEFINE(IdentifyProperties, IdentifyProperties_FIELDS)
MASON_STRUCT_DEFINE(IdentifyActivityButton, IdentifyActivityButton_FIELDS)
MASON_STRUCT_DEFINE(IdentifyActivity, IdentifyActivity_FIELDS)
MASON_STRUCT_DEFINE(IdentifyPresence, IdentifyPresence_FIELDS)
MASON_STRUCT_DEFINE(IdentifyEventData, IdentifyEvent_FIELDS)
MASON_STRUCT_DEFINE(GatewayEventPayload, GatewayEventPayload_FIELDS)

MASON_IMPL(IdentifyProperties, IdentifyProperties_FIELDS)
MASON_IMPL(IdentifyActivityButton, IdentifyActivityButton_FIELDS)
MASON_IMPL(IdentifyActivity, IdentifyActivity_FIELDS)
MASON_IMPL(IdentifyPresence, IdentifyPresence_FIELDS)
MASON_IMPL(IdentifyEventData, IdentifyEvent_FIELDS)
MASON_IMPL(GatewayEventPayload, GatewayEventPayload_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

int main(void) {
    int ret = 0;
    mason_init_default();

    size_t json_len = 0;
    char *json_str = mason_read_file_to_string("examples/data/discord.json", &json_len);
    if (!json_str) {
        fprintf(stderr, "Failed to read examples/data/discord.json\n");
        ret = 1;
        goto cleanup;
    }

    MASON_ArenaMark mark = mason_arena_mark(_mason_global_arena);

    GatewayEventPayload *payload = GatewayEventPayload_from_string_sized(json_str, json_len);
    if (!payload) {
        fprintf(stderr, "Error: %s\n", mason_error());
        ret = 1;
        goto cleanup;
    }

    MASON_ArenaMark after = mason_arena_mark(_mason_global_arena);
    size_t bytes_used = after.block->offset - mark.offset;

    printf("Parsing successful\n");
    printf("Arena bytes used: %zu\n", bytes_used);
    printf("Op code: %d\n", payload->op);

cleanup:
    free(json_str);
    mason_shutdown();
    return ret;
}
