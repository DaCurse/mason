# Mason

> C **Ma**cros for J**SON**.

Mason is a C library that generates boilerplate to convert JSON to C structs and vice versa.
It uses [cJSON](https://github.com/DaveGamble/cJSON) to do the actual JSON parsing/serialization.

I created this library for my own use, trying to reduce boilerplate code when dealing with JSON for my [Discord bot in C](https://github.com/DaCurse/muse).
The library is very WIP and definitely not production ready.

## What it does

Mason lets you declare C structs that map to JSON objects using X-macros to generate helper methods to parse/serialize them:

```c
#define MASON_PRINT_IMPL // Enable _print() functions
#include "mason.h"

// Define fields with an X-macro
#define User_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                              \
    FIELD(int32_t, age)                                              \
    ARRAY(string, tags)

// Declare the struct + function prototypes
MASON_STRUCT_DEFINE(User, User_FIELDS)

// Generate all implementations
MASON_IMPL(User, User_FIELDS)

int main(void) {
    User *u = User_from_string("{\"name\":\"Alice\",\"age\":30,\"tags\":[\"admin\"]}");
    // User is a normal C struct
    printf("Name: %s\nAge: %d\n", u->name, u->age);
    // You can pretty-print it with the print function
    User_print(u);
    // Mason structs are allocated on the heap and must be freed with the generated free function
    User_free(u);
}
```

### What it doesn't do yet

- Fields aren't required, if they are missing, their value in the struct is zeroed out
- No support for default values
- No support for arrays of mixed struct types.

## API overview

For a struct named `Foo`, `MASON_STRUCT_DEFINE` + `MASON_IMPL` generates:

| Function | Description |
| --- | --- |
| `Foo_from_string(const char *str)` | Parse a JSON string into a heap-allocated `Foo *` |
| `Foo_from_string_sized(const char *str, size_t len)` | Same, but with explicit length |
| `Foo_from_json(MASON_Parsed json)` | Parse from an already-parsed JSON handle |
| `Foo_to_json(Foo *obj)` | Serialize to a `MASON_Parsed` handle |
| `Foo_to_string(MASON_Parsed json)` | Convert a JSON handle to a `char *` (user frees) |
| `Foo_string_free(char *str)` | Free a string from `_to_string` |
| `Foo_free_json(MASON_Parsed json)` | Free a JSON handle returned by `_to_json` |
| `Foo_free(Foo *obj)` | Free the struct and all owned memory |
| `Foo_free_members(Foo *obj)` | Free owned memory without freeing the struct itself |
| `Foo_print(Foo *obj)` | Pretty-print (requires `MASON_PRINT_IMPL`) |

### Supported field types

| Macro | C type | Notes |
| --- | --- | --- |
| `FIELD(type, name)` | Any primitive | `int32_t`, `int64_t`, `double`, `string`, `bool` |
| `ARRAY(type, name)` | Typed array | Generates `type *name` + `size_t name_count` |
| `ARRAY_MULTI(name)` | Mixed array | Heterogeneous `MASON_RawValue` tagged union |
| `OBJECT(type, name)` | Nested struct | Pointer to another Mason struct |
| `ARRAY_OBJECT(type, name)` | Array of structs | Inline array (not pointer-to-pointer) |

> [!NOTE]
> `ARRAY_MULTI` won't parse objects/arrays into Mason structs/arrays of structs. They store deep-copied cJSON AST handles.

### Type aliases

If you have a type that's really just a primitive under the hood (like an enum), you can define `MASON_TYPE_ALIAS_##type` to treat it as that primitive.

```c
typedef enum { ROLE_USER, ROLE_ADMIN } Role;
#define MASON_TYPE_ALIAS_Role int32_t
```

Now you can use `FIELD(Role, role)` in your field list and Mason will parse/serialize it as an `int32_t`.

### Public helpers

These are the public helpers you'd most likely use, but there are others available.

| Function | Description |
| --- | --- |
| `mason_parse(const char *json_str)` | Parse JSON text into a `MASON_Parsed` handle |
| `mason_parse_sized(const char *json_str, size_t len)` | Parse with explicit length into a `MASON_Parsed` handle |
| `mason_parse_error(void)` | Get the backend's last parse error pointer |
| `mason_delete(MASON_Parsed json)` | Free a `MASON_Parsed` handle |
