#include "mason.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define DEFAULT_ARENA_CAPACITY    (1024 * 1024)
#define MASON_ERROR_CONTEXT_CHARS (12)

MASON_Arena *_mason_global_arena = NULL;
static const char *_mason_error_ptr = NULL;
static char _mason_error_buffer[512];
char _mason_path_buffer[256];
size_t _mason_path_len = 0;
const char *_mason_root_name = NULL;

/* Push a field name to the current parse path */
size_t _mason_path_push(const char *name) {
    size_t saved = _mason_path_len;
    size_t name_len = strlen(name);
    if (_mason_path_len + name_len + 2 < sizeof(_mason_path_buffer)) {
        memcpy(_mason_path_buffer + _mason_path_len, name, name_len);
        _mason_path_len += name_len;
        _mason_path_buffer[_mason_path_len++] = '.';
        _mason_path_buffer[_mason_path_len] = '\0';
    }
    return saved;
}

/* Push an array index to the current parse path */
size_t _mason_path_push_index(const char *name, size_t index) {
    size_t saved = _mason_path_len;
    char idx[32];
    int idx_len = snprintf(idx, sizeof(idx), "[%zu]", index);
    size_t name_len = strlen(name);
    if (idx_len > 0 && _mason_path_len + name_len + (size_t)idx_len + 2 < sizeof(_mason_path_buffer)) {
        memcpy(_mason_path_buffer + _mason_path_len, name, name_len);
        _mason_path_len += name_len;
        memcpy(_mason_path_buffer + _mason_path_len, idx, (size_t)idx_len);
        _mason_path_len += (size_t)idx_len;
        _mason_path_buffer[_mason_path_len++] = '.';
        _mason_path_buffer[_mason_path_len] = '\0';
    }
    return saved;
}

/* Pop to a previous index in the parse path  */
void _mason_path_pop(size_t saved) {
    _mason_path_len = saved;
    _mason_path_buffer[_mason_path_len] = '\0';
}

/* Bounded printf into `buf` with `len` starting at `used` with safe truncation */
static size_t _mason_append(char *buf, size_t len, size_t used, const char *fmt, ...) {
    if (!buf || len == 0) {
        return 0;
    }
    if (used >= len) {
        buf[len - 1] = '\0';
        return len - 1;
    }

    va_list ap;
    va_start(ap, fmt);
    int wrote = vsnprintf(buf + used, len - used, fmt, ap);
    va_end(ap);

    if (wrote < 0) {
        buf[used] = '\0';
        return used;
    }

    if ((size_t)wrote >= len - used) {
        buf[len - 1] = '\0';
        return len - 1;
    }

    return used + (size_t)wrote;
}

/* Walk from `start` to `pos` once to compute 1-based line/column for `pos` */
static size_t _mason_count_line_col(const char *start, const char *pos, size_t *col_out) {
    size_t line = 1;
    size_t col = 1;

    for (const char *p = start; p < pos && *p; p++) {
        if (*p == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }

    if (col_out) {
        *col_out = col;
    }
    return line;
}

/* Build a short, whitespace-normalized snippet around `pos` from `start` into `out` */
static void _mason_format_context_snippet(const char *start, const char *pos, char *out, size_t out_len) {
    if (!out || out_len == 0) {
        return;
    }

    out[0] = '\0';
    if (!start || !pos || pos < start) {
        return;
    }

    const size_t context = MASON_ERROR_CONTEXT_CHARS;
    const char *end = start;
    while (*end) {
        end++;
    }

    ptrdiff_t dist = pos - start;
    const char *left = dist > (ptrdiff_t)context ? pos - context : start;
    const char *right = pos + context;
    if (right > end) {
        right = end;
    }
    size_t len = 0;

    for (const char *p = left; p < right && len + 1 < out_len; p++) {
        char c = *p;
        if (c == '\n' || c == '\r' || c == '\t') {
            c = ' ';
        }
        out[len++] = c;
    }
    out[len] = '\0';
}

static const char *_mason_cjson_type_name(const MASON_Parsed item) {
    if (!item)
        return "missing";

    if (cJSON_IsNull(item))
        return "null";
    if (cJSON_IsBool(item))
        return "bool";
    if (cJSON_IsString(item))
        return "string";
    if (cJSON_IsNumber(item))
        return "number";
    if (cJSON_IsArray(item))
        return "array";
    if (cJSON_IsObject(item))
        return "object";

    return "unknown";
}

/* Format a human-readable parse error for `json_str` and `err` into `_mason_error_buffer` */
void _mason_set_parse_error(const char *json_str, const char *err, bool null_input) {
    _mason_error_buffer[0] = '\0';

    if (null_input) {
        _mason_append(_mason_error_buffer,
                      sizeof(_mason_error_buffer),
                      0,
                      "JSON parse error: null input");
        _mason_error_ptr = _mason_error_buffer;
        return;
    }

    if (!json_str) {
        _mason_error_ptr = NULL;
        return;
    }

    if (!err || err < json_str) {
        char snippet[64];
        _mason_format_context_snippet(json_str, json_str, snippet, sizeof(snippet));
        size_t used = _mason_append(_mason_error_buffer,
                                    sizeof(_mason_error_buffer),
                                    0,
                                    "JSON parse error: unknown location");
        if (snippet[0]) {
            _mason_append(_mason_error_buffer,
                          sizeof(_mason_error_buffer),
                          used,
                          ": %s",
                          snippet);
        }
        _mason_error_ptr = _mason_error_buffer;
        return;
    }

    size_t col = 1;
    size_t line = _mason_count_line_col(json_str, err, &col);

    char snippet[64];
    _mason_format_context_snippet(json_str, err, snippet, sizeof(snippet));

    _mason_append(_mason_error_buffer,
                  sizeof(_mason_error_buffer),
                  0,
                  "JSON parse error at line %zu, col %zu: %s",
                  line,
                  col,
                  snippet[0] ? snippet : "(no context)");

    _mason_error_ptr = _mason_error_buffer;
}

/* Format human-redaable validation error including root object name, erroneous field path, expected and present type */
void _mason_set_validation_error(const char *name, const char *type_name, const char *resolved_type_name, MASON_Parsed item) {
    _mason_error_buffer[0] = '\0';

    const char *field_name = (name && name[0]) ? name : "(unknown)";
    const char *actual = _mason_cjson_type_name(item);

    bool is_aliased = type_name && resolved_type_name && strcmp(type_name, resolved_type_name) != 0;
    size_t used = _mason_append(_mason_error_buffer,
                                sizeof(_mason_error_buffer),
                                0,
                                "JSON validation error (%s): field \"%s%s\" expected ",
                                _mason_root_name ? _mason_root_name : "unknown",
                                _mason_path_len > 0 ? _mason_path_buffer : "",
                                field_name);

    if (is_aliased) {
        used = _mason_append(_mason_error_buffer,
                             sizeof(_mason_error_buffer),
                             used,
                             "%s (%s)",
                             type_name,
                             resolved_type_name);
    } else {
        used = _mason_append(_mason_error_buffer,
                             sizeof(_mason_error_buffer),
                             used,
                             "%s",
                             type_name);
    }

    _mason_append(_mason_error_buffer,
                  sizeof(_mason_error_buffer),
                  used,
                  ", got %s",
                  actual);

    _mason_error_ptr = _mason_error_buffer;
}

const char *mason_error(void) {
    return _mason_error_ptr;
}

void mason_bind_global_arena(MASON_Arena *a) {
    assert(a != NULL);
    if (a) {
        _mason_global_arena = a;
    }
}

static void *mason_cjson_malloc(size_t size) {
    return mason_arena_alloc(_mason_global_arena, size);
}

static void mason_cjson_free(void *ptr) { (void)ptr; }

void mason_init(void) {
    assert(_mason_global_arena != NULL);

    cJSON_Hooks hooks = {
        .malloc_fn = mason_cjson_malloc,
        .free_fn = mason_cjson_free,
    };
    cJSON_InitHooks(&hooks);
}

void mason_init_default(void) {
    MASON_Arena *arena = mason_arena_create(DEFAULT_ARENA_CAPACITY);
    assert(arena != NULL);
    if (!arena)
        return;
    mason_bind_global_arena(arena);
    mason_init();
}

void mason_reset(void) {
    assert(_mason_global_arena != NULL);
    if (_mason_global_arena) {
        mason_arena_reset(_mason_global_arena);
    }
}

void mason_shutdown(void) {
    assert(_mason_global_arena != NULL);
    cJSON_InitHooks(NULL);
    if (_mason_global_arena) {
        mason_arena_destroy(_mason_global_arena);
        _mason_global_arena = NULL;
    }
}
