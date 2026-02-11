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
- No support for arrays of mixed struct types.

### Supported field types

| Macro | C type | Notes |
| --- | --- | --- |
| `FIELD(type, name)` | Any primitive | `int32_t`, `int64_t`, `double`, `string`, `bool` |
| `ARRAY(type, name)` | Typed array | Generates `type *name` + `size_t name_count` |
| `ARRAY_MULTI(name)` | Mixed array | Heterogeneous `MASON_RawValue` tagged union |
| `OBJECT(type, name)` | Nested struct | Pointer to another Mason struct |
| `ARRAY_OBJECT(type, name)` | Array of structs | Inline array (not pointer-to-pointer) |

> [!NOTE]
> `ARRAY_MULTI` won't parse objects/arrays into Mason structs/arrays of structs. They store deep-copied backend AST handles.

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

If you have a type that's really just a primitive under the hood (like an enum), you can define `MASON_TYPE_ALIAS_##type` to treat it as that primitive.

```c
typedef enum { ROLE_USER, ROLE_ADMIN } Role;
#define MASON_TYPE_ALIAS_Role int32_t
```

Now you can use `FIELD(Role, role)` in your field list and Mason will parse/serialize it as an `int32_t`.

## API overview

For a struct named `Foo`, `MASON_STRUCT_DEFINE` + `MASON_IMPL` generates:

| Function | Description |
| --- | --- |
| `Foo_from_json(const char *str)` | Parse a JSON string into a heap-allocated `Foo *` |
| `Foo_from_json_sized(const char *str, size_t len)` | Same, but with explicit length |
| `Foo_from_parsed(MASON_Parsed json)` | Parse from an already-parsed JSON handle |
| `Foo_to_json(Foo *obj)` | Serialize to a `MASON_Parsed` handle |
| `Foo_to_string(MASON_Parsed json)` | Convert a JSON handle to a `char *` (user frees) |
| `Foo_string_free(char *str)` | Free a string from `_to_string` |
| `Foo_free_json(MASON_Parsed json)` | Free a JSON handle returned by `_to_json` |
| `Foo_free(Foo *obj)` | Free the struct and all owned memory |
| `Foo_free_members(Foo *obj)` | Free owned memory without freeing the struct itself |
| `Foo_print(Foo *obj)` | Pretty-print (requires `MASON_PRINT_IMPL`) |

### Public helpers

There are other public helper functions, but these are the ones you'd most likely use.

| Function | Description |
| --- | --- |
| `mason_parse(const char *json_str)` | Parse JSON text into a `MASON_Parsed` handle |
| `mason_parse_sized(const char *json_str, size_t len)` | Parse with explicit length into a `MASON_Parsed` handle |
| `mason_parse_error(void)` | Get the backend's last parse error pointer |
| `mason_delete(MASON_Parsed json)` | Free a `MASON_Parsed` handle |

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

Here's the full list, using the [cJSON backend](backends/cjson.h) as an example.

Notes:

- The backend abstraction assumes a lifetime management model similar to cJSON. More details in table below.
- The JSON spec permits representing numbers as IEEE-754 doubles, so integers beyond $2^{53}$ can lose precision on parse/round-trip.
- `_MASON_BACKEND_JSON_T` must be a pointer-like handle type. All backend macros operate on and return this type directly (not pointers to it).
- Backends must ensure that `_MASON_BACKEND_GET_*` and `_MASON_BACKEND_ARRAY_GET` return pointers that remain valid until `_MASON_BACKEND_DELETE()` is called on the `_MASON_BACKEND_JSON_T` instance.

| Macro | cJSON example | Lifetime/ownership notes |
| --- | --- | --- |
| `_MASON_BACKEND_JSON_T` | `cJSON*` | Opaque backend JSON handle (node or root). Owned by caller unless stated otherwise. |
| `_MASON_BACKEND_PARSE(str)` | `cJSON_Parse(str)` | Parses text into an owned backend node. |
| `_MASON_BACKEND_PARSE_SIZED(s, len)` | `cJSON_ParseWithLength(s, len)` | Parses text into an owned backend node. |
| `_MASON_BACKEND_PARSE_ERROR()` | `cJSON_GetErrorPtr()` | Returns pointer to static error info. |
| `_MASON_BACKEND_GET_FIELD(json, name)` | `cJSON_GetObjectItemCaseSensitive(json, name)` | Field lookup returns a borrowed node pointer. |
| `_MASON_BACKEND_IS_int32_t(item)` | `cJSON_IsNumber(item)` | |
| `_MASON_BACKEND_IS_int64_t(item)` | `cJSON_IsNumber(item)` | |
| `_MASON_BACKEND_IS_double(item)` | `cJSON_IsNumber(item)` | |
| `_MASON_BACKEND_IS_string(item)` | `(cJSON_IsString(item) && (item)->valuestring)` | |
| `_MASON_BACKEND_IS_bool(item)` | `cJSON_IsBool(item)` | |
| `_MASON_BACKEND_IS_ARRAY(item)` | `cJSON_IsArray(item)` | |
| `_MASON_BACKEND_IS_OBJECT(item)` | `cJSON_IsObject(item)` | |
| `_MASON_BACKEND_IS_NULL(item)` | `cJSON_IsNull(item)` | |
| `_MASON_BACKEND_GET_int32_t(item)` | `((int32_t)(item)->valueint)` | Reads scalar into a struct field. |
| `_MASON_BACKEND_GET_int64_t(item)` | `((int64_t)(item)->valuedouble)` | Reads scalar into a struct field. |
| `_MASON_BACKEND_GET_double(item)` | `((item)->valuedouble)` | Reads scalar into a struct field. |
| `_MASON_BACKEND_GET_string(item)` | `((item)->valuestring)` | Reads a borrowed string pointer.  |
| `_MASON_BACKEND_GET_bool(item)` | `cJSON_IsTrue(item)` | Reads scalar into a struct field. |
| `_MASON_BACKEND_ARRAY_SIZE(arr)` | `cJSON_GetArraySize(arr)` | Reads array length. |
| `_MASON_BACKEND_ARRAY_GET(arr, idx)` | `cJSON_GetArrayItem(arr, idx)` | Reads a borrowed element pointer. |
| `_MASON_BACKEND_CREATE_OBJECT()` | `cJSON_CreateObject()` | Creates an owned object node from struct data. |
| `_MASON_BACKEND_CREATE_ARRAY()` | `cJSON_CreateArray()` | Creates an owned array node from struct data. |
| `_MASON_BACKEND_CREATE_int32_t(v)` | `cJSON_CreateNumber(v)` | Creates an owned scalar node from a struct value. |
| `_MASON_BACKEND_CREATE_int64_t(v)` | `cJSON_CreateNumber((double)(v))` | Creates an owned scalar node from a struct value. |
| `_MASON_BACKEND_CREATE_double(v)` | `cJSON_CreateNumber(v)` | Creates an owned scalar node from a struct value. |
| `_MASON_BACKEND_CREATE_string(v)` | `cJSON_CreateString(v)` | Creates an owned string node from a struct value. |
| `_MASON_BACKEND_CREATE_bool(v)` | `cJSON_CreateBool(v)` | Creates an owned bool node from a struct value. |
| `_MASON_BACKEND_CREATE_NULL()` | `cJSON_CreateNull()` | Creates an owned null node. |
| `_MASON_BACKEND_OBJECT_ADD(obj, key, val)` | `cJSON_AddItemToObject(obj, key, val)` | `val` becomes owned by `obj`. |
| `_MASON_BACKEND_ARRAY_APPEND(arr, val)` | `cJSON_AddItemToArray(arr, val)` | `val` becomes owned by `arr`. |
| `_MASON_BACKEND_DUPLICATE(node)` | `cJSON_Duplicate((node), 1)` | Creates an owned deep copy. |
| `_MASON_BACKEND_DELETE(json)` | `cJSON_Delete(json)` | Frees owned backend nodes. |
| `_MASON_BACKEND_TO_STRING(json)` | `cJSON_Print(json)` | Returns an allocated string. |
| `_MASON_BACKEND_STRING_FREE(str)` | `free(str)` | Frees strings returned by `_MASON_BACKEND_TO_STRING`. |
