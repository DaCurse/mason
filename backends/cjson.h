#ifndef MASON_BACKEND_CJSON_H
#define MASON_BACKEND_CJSON_H

#ifndef _MASON_BACKEND
#define _MASON_BACKEND
#include <cjson/cJSON.h>
#include <stdint.h>

/* Types */
#define _MASON_BACKEND_JSON_T                    cJSON

/* Parsing */
#define _MASON_BACKEND_PARSE(str)                cJSON_Parse(str)
#define _MASON_BACKEND_PARSE_SIZED(s, len)       cJSON_ParseWithLength(s, len)
#define _MASON_BACKEND_PARSE_ERROR()             cJSON_GetErrorPtr()

/* Field access */
#define _MASON_BACKEND_GET_FIELD(json, name)     cJSON_GetObjectItemCaseSensitive(json, name)

/* Type checks */
#define _MASON_BACKEND_IS_int32_t(item)          cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_int64_t(item)          cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_double(item)           cJSON_IsNumber(item)
#define _MASON_BACKEND_IS_string(item)           (cJSON_IsString(item) && (item)->valuestring)
#define _MASON_BACKEND_IS_bool(item)             cJSON_IsBool(item)
#define _MASON_BACKEND_IS_ARRAY(item)            cJSON_IsArray(item)
#define _MASON_BACKEND_IS_OBJECT(item)           cJSON_IsObject(item)
#define _MASON_BACKEND_IS_NULL(item)             cJSON_IsNull(item)

/* Value getters */
#define _MASON_BACKEND_GET_int32_t(item)         ((int32_t)(item)->valueint)
#define _MASON_BACKEND_GET_int64_t(item)         ((int64_t)(item)->valuedouble)
#define _MASON_BACKEND_GET_double(item)          ((item)->valuedouble)
#define _MASON_BACKEND_GET_string(item)          ((item)->valuestring)
#define _MASON_BACKEND_GET_bool(item)            cJSON_IsTrue(item)

/* Array access */
#define _MASON_BACKEND_ARRAY_SIZE(arr)           cJSON_GetArraySize(arr)
#define _MASON_BACKEND_ARRAY_GET(arr, idx)       cJSON_GetArrayItem(arr, idx)

/* Node creation */
#define _MASON_BACKEND_CREATE_OBJECT()           cJSON_CreateObject()
#define _MASON_BACKEND_CREATE_ARRAY()            cJSON_CreateArray()
#define _MASON_BACKEND_CREATE_int32_t(v)         cJSON_CreateNumber(v)
#define _MASON_BACKEND_CREATE_int64_t(v)         cJSON_CreateNumber((double)(v))
#define _MASON_BACKEND_CREATE_double(v)          cJSON_CreateNumber(v)
#define _MASON_BACKEND_CREATE_string(v)          cJSON_CreateString(v)
#define _MASON_BACKEND_CREATE_bool(v)            cJSON_CreateBool(v)
#define _MASON_BACKEND_CREATE_NULL()             cJSON_CreateNull()

/* Tree manipulation */
#define _MASON_BACKEND_OBJECT_ADD(obj, key, val) cJSON_AddItemToObject(obj, key, val)
#define _MASON_BACKEND_ARRAY_APPEND(arr, val)    cJSON_AddItemToArray(arr, val)

/* Cleanup */
#define _MASON_BACKEND_DELETE(json)              cJSON_Delete(json)
#define _MASON_BACKEND_TO_STRING(json)           cJSON_Print(json)
#define _MASON_BACKEND_STRING_FREE(str)          free(str)

#endif // _MASON_BACKEND

#endif // MASON_BACKEND_CJSON_H
