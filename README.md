# Mason

> C **Ma**cros for J**SON**.

I created this library to try and reduce boilerplate code when working with JSON in C.
The library is very WIP and definitely not production ready.

## What it does

Mason lets you declare C structs that map to JSON objects using X-macros. From a single field list you get:

- **Parsing** - JSON string to C struct, with nested objects and arrays
- **Serialization** - C struct to JSON string
- **Pretty printing** - human-readable dump of a struct (opt-in via `MASON_PRINT_IMPL`)
- **Memory management** - `_free()` that cleans up everything, including nested objects and string arrays
- **Type aliases** - map your own types (like an enum) to a primitive with one `#define`
- **Backend abstraction** - the JSON library is behind a macro layer, so you can swap it without touching your code

### What is currently missing

- Fields aren't required, if they are missing, their value in the struct is zeroed out
- No support for default values
- No support for arrays of mixed non-primitive types (e.g. array of objects)

### Supported field types

| Macro | C type | Notes |
| --- | --- | --- |
| `FIELD(type, name)` | Any primitive | `int32_t`, `int64_t`, `double`, `string`, `bool` |
| `ARRAY(type, name)` | Typed array | Generates `type *name` + `size_t name_count` |
| `ARRAY_MULTI(name)` | Mixed array | Heterogeneous `Mason_RawValue` tagged union |
| `OBJECT(type, name)` | Nested struct | Pointer to another Mason struct |
| `ARRAY_OBJECT(type, name)` | Array of structs | Inline array (not pointer-to-pointer) |

> [!NOTE]
> Mixed arrays don't parse objects/arrays, they just store the backend's AST nodes as-is.

## Quick start

```c
#define MASON_PRINT_IMPL   // enable _print() functions
#define MASON_USE_CJSON    // pick your backend
#include "mason.h"

// define fields with an X-macro
#define USER_FIELDS(FIELD, ARRAY, ARRAY_MULTI, OBJECT, ARRAY_OBJECT) \
    FIELD(string, name)                                              \
    FIELD(int32_t, age)                                              \
    ARRAY(string, tags)

// declare the struct + function prototypes
MASON_STRUCT_DEFINE(User, USER_FIELDS)

// generate all implementations
MASON_IMPL(User, USER_FIELDS)

int main(void) {
    User *u = User_from_json("{\"name\":\"Alice\",\"age\":30,\"tags\":[\"admin\"]}");
    User_print(u);
    User_free(u);
}
```

This gives you `User_from_json`, `User_to_json`, `User_print`, `User_free` [and more](#api-overview), all generated from the one field list.

## Type aliases

If you have a type that's really just a primitive under the hood (like an enum), you can use `MASON_BASE_TYPE_##type` to treat it as that primitive.

```c
typedef enum { ROLE_USER, ROLE_ADMIN } Role;
#define MASON_BASE_TYPE_Role int32_t
```

Now you can use `FIELD(Role, role)` in your field list and Mason will parse/serialize it as an `int32_t`.

## API overview

For a struct named `Foo`, `MASON_STRUCT_DEFINE` + `MASON_IMPL` generates:

| Function | Description |
| --- | --- |
| `Foo_from_json(const char *str)` | Parse a JSON string into a heap-allocated `Foo *` |
| `Foo_from_json_sized(const char *str, size_t len)` | Same, but with explicit length |
| `Foo_from_parsed(MASON_Parsed *json)` | Parse from an already-parsed JSON tree |
| `Foo_to_json(Foo *obj)` | Serialize to a `MASON_Parsed *` tree |
| `Foo_to_string(MASON_Parsed *json)` | Convert a JSON tree to a `char *` (caller frees) |
| `Foo_string_free(char *str)` | Free a string from `_to_string` |
| `Foo_free(Foo *obj)` | Free the struct and all owned memory |
| `Foo_free_members(Foo *obj)` | Free owned memory without freeing the struct itself |
| `Foo_print(Foo *obj)` | Pretty-print (requires `MASON_PRINT_IMPL`) |

## Implementing a backend

To implement a custom JSON library backend, create a header that:

1. Guards with `#ifndef _MASON_BACKEND` / `#define _MASON_BACKEND` so only one backend is active
2. Defines all the required macros listed below

Then include it before `mason.h`:

```c
#include "backends/mylib.h"
#include "mason.h"
```

### Required macros

Here's the full list, using the [cJSON backend](backends/cjson.h) as a reference.

> [!NOTE]
> The backend abstraction assumes a lifetime management model similar to cJSON.

```c
/* The underlying JSON node type */
#define _MASON_BACKEND_JSON_T              cJSON

/* Parsing */
#define _MASON_BACKEND_PARSE(str)          cJSON_Parse(str)
#define _MASON_BACKEND_PARSE_SIZED(s, len) cJSON_ParseWithLength(s, len)
#define _MASON_BACKEND_PARSE_ERROR()       cJSON_GetErrorPtr()

/* Field access */
#define _MASON_BACKEND_GET_FIELD(json, name) cJSON_GetObjectItemCaseSensitive(json, name)

/* Type checks - one per primitive */
#define _MASON_BACKEND_IS_int32_t(item)    cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_int64_t(item)    cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_double(item)     cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_string(item)     (cJSON_IsString(item) && (item)->valuestring)
#define _MASON_BACKEND_IS_bool(item)       cJSON_IsBool(item)
#define _MASON_BACKEND_IS_ARRAY(item)      cJSON_IsArray(item)
#define _MASON_BACKEND_IS_OBJECT(item)     cJSON_IsObject(item)
#define _MASON_BACKEND_IS_NULL(item)       cJSON_IsNull(item)

/* Value getters */
#define _MASON_BACKEND_GET_int32_t(item)   ((int32_t)(item)->valueint)
#define _MASON_BACKEND_GET_int64_t(item)   ((int64_t)(item)->valuedouble)
#define _MASON_BACKEND_GET_double(item)    ((item)->valuedouble)
#define _MASON_BACKEND_GET_string(item)    ((item)->valuestring)
#define _MASON_BACKEND_GET_bool(item)      cJSON_IsTrue(item)

/* Array access */
#define _MASON_BACKEND_ARRAY_SIZE(arr)     cJSON_GetArraySize(arr)
#define _MASON_BACKEND_ARRAY_GET(arr, idx) cJSON_GetArrayItem(arr, idx)

/* Node creation */
#define _MASON_BACKEND_CREATE_OBJECT()     cJSON_CreateObject()
#define _MASON_BACKEND_CREATE_ARRAY()      cJSON_CreateArray()
#define _MASON_BACKEND_CREATE_int32_t(v)   cJSON_CreateNumber(v)
#define _MASON_BACKEND_CREATE_int64_t(v)   cJSON_CreateNumber((double)(v))
#define _MASON_BACKEND_CREATE_double(v)    cJSON_CreateNumber(v)
#define _MASON_BACKEND_CREATE_string(v)    cJSON_CreateString(v)
#define _MASON_BACKEND_CREATE_bool(v)      cJSON_CreateBool(v)
#define _MASON_BACKEND_CREATE_NULL()       cJSON_CreateNull()

/* Tree manipulation */
#define _MASON_BACKEND_OBJECT_ADD(obj, key, val) cJSON_AddItemToObject(obj, key, val)
#define _MASON_BACKEND_ARRAY_APPEND(arr, val)    cJSON_AddItemToArray(arr, val)

/* Cleanup */
#define _MASON_BACKEND_DELETE(json)        cJSON_Delete(json)
#define _MASON_BACKEND_TO_STRING(json)     cJSON_Print(json)
#define _MASON_BACKEND_STRING_FREE(str)    free(str)
```
