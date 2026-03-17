#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../mason.h"

#define ITERATIONS  (2000)
#define RESET_EVERY (25)

#define Address_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, street, NULLABLE)                        \
    FIELD(int32_t, zip, REQUIRED)

#define Person_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name, REQUIRED)                         \
    FIELD(int64_t, id, REQUIRED)                          \
    FIELD(double, score, REQUIRED)                        \
    FIELD(bool, active, REQUIRED)                         \
    ARRAY(string, tags)                                   \
    OBJECT(Address, address)                              \
    ARRAY_OBJECT(Address, history)

#define Report_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    OBJECT(Person, owner)                                 \
    ARRAY_OBJECT(Person, people)

MASON_STRUCT_DEFINE(Address, Address_FIELDS)
MASON_STRUCT_DEFINE(Person, Person_FIELDS)
MASON_STRUCT_DEFINE(Report, Report_FIELDS)

MASON_IMPL(Address, Address_FIELDS)
MASON_IMPL(Person, Person_FIELDS)
MASON_IMPL(Report, Report_FIELDS)

char *mason_read_file_to_string(const char *path, size_t *out_len);

static bool arena_is_fully_reset(MASON_Arena *arena) {
    if (!arena || !arena->head || arena->current != arena->head || arena->head->offset != 0) {
        return false;
    }

    for (MASON_ArenaBlock *block = arena->head->next; block; block = block->next) {
        if (block->offset != 0) {
            return false;
        }
    }
    return true;
}

static bool validate_report(const Report *report) {
    if (!report || !report->owner) {
        return false;
    }

    if (strcmp(report->owner->name, "owner") != 0 || report->owner->id != 999) {
        return false;
    }

    if (report->owner->tags_count != 3 || report->people_count != 2) {
        return false;
    }

    if (!report->owner->address || report->owner->address->zip != 11111) {
        return false;
    }

    if (strcmp(report->people[0].name, "alice") != 0 || report->people[1].id != 202) {
        return false;
    }

    return true;
}

int main(void) {
    int ret = 1;
    size_t json_len = 0;
    char *json_str = NULL;
    size_t peak_total_used = 0;
    size_t peak_total_capacity = 0;
    size_t peak_block_count = 0;
    int peak_iteration = 0;

    mason_init_default();

    json_str = mason_read_file_to_string("examples/data/arena.json", &json_len);
    if (!json_str) {
        fprintf(stderr, "failed to read examples/data/arena.json\n");
        goto cleanup;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        Report *report = NULL;
        MASON_Parsed round_trip = NULL;
        string compact = NULL;

        report = Report_from_string_sized(json_str, json_len);
        if (!report) {
            fprintf(stderr, "mason failed at iter %d: %s\n", i, mason_error());
            goto cleanup;
        }

        if (!validate_report(report)) {
            fprintf(stderr, "validation failed at iter %d\n", i);
            goto cleanup;
        }

        round_trip = Report_to_json(report);
        if (!round_trip) {
            fprintf(stderr, "serialize failed at iter %d\n", i);
            goto cleanup;
        }

        compact = Report_to_string(round_trip, false);
        if (!compact || strstr(compact, "\"owner\"") == NULL) {
            fprintf(stderr, "to_string failed at iter %d\n", i);
            goto cleanup;
        }

        MASON_ArenaStats arena_stats = mason_arena_stats(_mason_global_arena);
        if (arena_stats.total_used > peak_total_used) {
            peak_total_used = arena_stats.total_used;
            peak_total_capacity = arena_stats.total_capacity;
            peak_block_count = arena_stats.block_count;
            peak_iteration = i + 1;
        }

        if ((((i + 1) % RESET_EVERY) == 0) || (i == (ITERATIONS - 1))) {
            mason_reset();

            if (!arena_is_fully_reset(_mason_global_arena)) {
                fprintf(stderr, "arena reset invariant failed at iter %d\n", i);
                goto cleanup;
            }
        }
    }

    printf("arena peak: iter=%d total_used=%zu total_capacity=%zu blocks=%zu\n",
           peak_iteration,
           peak_total_used,
           peak_total_capacity,
           peak_block_count);
    printf("arena regression passed (%d iterations)\n", ITERATIONS);
    ret = 0;

cleanup:
    free(json_str);
    mason_shutdown();
    return ret;
}
