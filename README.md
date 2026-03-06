# Mason

> C **Ma**cros for J**SON**.

Mason is a C library that generates boilerplate to convert JSON to C structs and vice versa.
It uses [cJSON](https://github.com/DaveGamble/cJSON) to do the actual JSON parsing/serialization.

I created this library for my own use, trying to reduce boilerplate code when dealing with JSON for my [Discord bot in C](https://github.com/DaCurse/muse).
The library is very WIP and definitely not production ready.

## What it does

Mason lets you declare C structs that map to JSON objects using X-macros to generate helper methods to parse/serialize them.

All allocations are arena-backed. You can create a default arena or create a custom one and bind it, and use the arena API to reset/rewind as needed.

```c
#include "mason.h"

// Define fields with an X-macro
#define User_FIELDS(FIELD, ARRAY, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                 \
    FIELD(int32_t, age)                                 \
    ARRAY(string, tags)

// Declare the struct + function prototypes
MASON_STRUCT_DEFINE(User, User_FIELDS)

// Generate all implementations
MASON_IMPL(User, User_FIELDS)

int main(void) {
    // Create a custom arena and bind it
    MASON_Arena *arena = mason_arena_create(1024 * 1024);
    if (!arena) {
        return 1;
    }
    mason_bind_global_arena(arena);
    mason_init();

    MASON_ArenaMark mark = mason_arena_mark(arena);
    User *u = User_from_string("{\"name\":\"Alice\",\"age\":30,\"tags\":[\"admin\"]}");
    if (!u) {
        printf("Error: %s\n", mason_error());
        mason_shutdown();
        return 1;
    }

    // User is a normal C struct
    printf("Name: %s\nAge: %d\n", u->name, u->age);
    // Rewind to a known point (u is now invalid)
    mason_arena_rewind(arena, mark);
    // Allocate a new User, reusing the same memory
    (void)User_from_string("{\"name\":\"Bob\",\"age\":42,\"tags\":[]}");

    // Reset the entire arena (essentially rewinding to the beginning)
    mason_reset();

    // Free the arena and unhook cJSON
    mason_shutdown();
    return 0;
}
```

### What it doesn't do yet

- No support for optional/nullable fields, all fields are required.
- No support for default values.
- No support for arrays of mixed types.
- No thread safety.

## API overview

For a struct named `Foo`, `MASON_STRUCT_DEFINE` + `MASON_IMPL` generates:

| Function                                             | Description                                                                                                                                      |
| ---------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| `Foo_from_string(const char *str)`                   | Parse a JSON string into an arena-backed `Foo *`                                                                                                 |
| `Foo_from_string_sized(const char *str, size_t len)` | Same, but with explicit length                                                                                                                   |
| `Foo_from_json(MASON_Parsed json)`                   | Parse from an already-parsed JSON handle                                                                                                         |
| `Foo_parse_into(Foo *obj, MASON_Parsed json)`        | Parse into an existing struct, returns `false` on failure. Caller must reset `_mason_path_len = 0` if calling directly (not via `Foo_from_json`) |
| `Foo_to_json(Foo *obj)`                              | Serialize to a `MASON_Parsed` handle                                                                                                             |
| `Foo_to_string(MASON_Parsed json)`                   | Convert a JSON handle to a null-terminated `char *` (arena-backed)                                                                               |

### Supported field types

| Macro                      | C type           | Notes                                            |
| -------------------------- | ---------------- | ------------------------------------------------ |
| `FIELD(type, name)`        | Any primitive    | `int32_t`, `int64_t`, `double`, `string`, `bool` |
| `ARRAY(type, name)`        | Typed array      | Generates `type *name` + `size_t name_count`     |
| `OBJECT(type, name)`       | Nested struct    | Pointer to another Mason struct                  |
| `ARRAY_OBJECT(type, name)` | Array of structs | Inline array (not pointer-to-pointer)            |

### Type aliases

If you have a type that's really just a primitive under the hood (like an enum), you can define `MASON_TYPE_ALIAS_##type` to treat it as that primitive.

```c
typedef enum { ROLE_USER, ROLE_ADMIN } Role;
#define MASON_TYPE_ALIAS_Role int32_t
```

Now you can use `FIELD(Role, role)` in your field list and Mason will parse/serialize it as an `int32_t`.

### Mason API

| Function                                  | Description                                                                                                                                                      |
| ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `mason_bind_global_arena(MASON_Arena *a)` | Use a caller-provided arena for all allocations                                                                                                                  |
| `mason_init(void)`                        | Initialize cJSON hooks for arena-backed allocations. Should only be called once after a global arena was bound and before any Mason generated functions are used |
| `mason_init_default(void)`                | Create and bind a default global arena, then initialize cJSON hooks                                                                                              |
| `mason_reset(void)`                       | Reset the global arena to reclaim all allocations                                                                                                                |
| `mason_shutdown(void)`                    | Destroy (free) the global arena and unhook cJSON                                                                                                                 |
| `mason_error(void)`                       | Get the last parse or validation error message                                                                                                                   |

### Error messages

`mason_error()` returns a human-readable string for both parse and validation failures.

**Parse errors** include a line/column and a short context snippet:

```
JSON parse error at line 1, col 17: me": "bob",}
```

**Validation errors** include the root struct name, the full field path, the expected type, and the actual type:

```
JSON validation error (User): field "prev_addresses[0].zip" expected int32_t, got bool
```

For aliased types (e.g. `#define MASON_TYPE_ALIAS_Role int32_t`) the message shows both names: `expected Role (int32_t)`.

### Arena API

This is what Mason uses internally. You can create and bind your own arena and use these helpers directly for finer-grained control, mainly to take advantage of marks and rewinding.

| Function                                                            | Description                                     |
| ------------------------------------------------------------------- | ----------------------------------------------- |
| `mason_arena_create(size_t initial_capacity)`                       | Create a new arena with an initial block size   |
| `mason_arena_alloc(MASON_Arena *arena, size_t size)`                | Allocate `size` bytes from the arena            |
| `mason_arena_calloc(MASON_Arena *arena, size_t count, size_t size)` | Allocate and zero `count * size` bytes          |
| `mason_arena_mark(MASON_Arena *arena)`                              | Capture a rewind point for the arena            |
| `mason_arena_rewind(MASON_Arena *arena, MASON_ArenaMark mark)`      | Rewind allocations back to a mark               |
| `mason_arena_reset(MASON_Arena *arena)`                             | Reset all allocations in the arena              |
| `mason_arena_stats(const MASON_Arena *arena)`                       | Get total usage and capacity info for the arena |
| `mason_arena_destroy(MASON_Arena *arena)`                           | Free all blocks and destroy the arena           |
