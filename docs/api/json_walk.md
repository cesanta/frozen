---
title: json_walk()
---

```c
/* JSON token type */
enum json_token_type {
  JSON_TYPE_INVALID = 0, /* memsetting to 0 should create INVALID value */
  JSON_TYPE_STRING,
  JSON_TYPE_NUMBER,
  JSON_TYPE_TRUE,
  JSON_TYPE_FALSE,
  JSON_TYPE_NULL,
  JSON_TYPE_OBJECT_START,
  JSON_TYPE_OBJECT_END,
  JSON_TYPE_ARRAY_START,
  JSON_TYPE_ARRAY_END,

  JSON_TYPES_CNT,
};

/*
 * Structure containing token type and value. Used in `json_walk()` and
 * `json_scanf()` with the format specifier `%T`.
 */
struct json_token {
  const char *ptr;           /* Points to the beginning of the value */
  int len;                   /* Value length */
  enum json_token_type type; /* Type of the token, possible values are above */
};

/* Callback-based API */
typedef void (*json_walk_callback_t)(void *callback_data,
                                     const char *name, size_t name_len,
                                     const char *path,
                                     const struct json_token *token);

/*
 * Parse `json_string`, invoking `callback` in a way similar to SAX parsers;
 * see `json_walk_callback_t`.
 */
int json_walk(const char *json_string, int json_string_length,
              json_walk_callback_t callback, void *callback_data);
```

`json_walk()` is a low-level, callback based parsing API.
`json_walk()` calls a given callback function for each scanned value.

Callback receives a name, a path to the value, a JSON token that points to the
value and an arbitrary user data pointer.

The path is constructed using this rule:
- Root element has "" (empty string) path
- When an object starts, `.` (dot) is appended to the path
- When an object key is parsed, a key name is appended to the path
- When an array is parsed, an `[ELEMENT_INDEX]` is appended for each element

For example, consider the following json string:
`{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`.
The sequence of callback invocations will be as follows:
- type: `JSON_TYPE_OBJECT_START`, name: `NULL`, path: `""`, value: `NULL`
- type: `JSON_TYPE_NUMBER`, name: `"foo"`, path: `".foo"`, value: `"123"`
- type: `JSON_TYPE_ARRAY_START`, name: `"bar"`, path: `".bar"`, value: `NULL`
- type: `JSON_TYPE_NUMBER`, name: `"0"`, path: `".bar[0]"`, value: `"1"`
- type: `JSON_TYPE_NUMBER`, name: `"1"`, path: `".bar[1]"`, value: `"2"`
- type: `JSON_TYPE_OBJECT_START`, name: `"2"`, path: `".bar[2]"`, value: `NULL`
- type: `JSON_TYPE_TRUE`, name: `"baz"`, path: `".bar[2].baz"`, value: `"true"`
- type: `JSON_TYPE_OBJECT_END`, name: `NULL`, path: `".bar[2]"`, value: `"{ \"baz\": true }"`
- type: `JSON_TYPE_ARRAY_END`, name: `NULL`, path: `".bar"`, value: `"[ 1, 2, { \"baz\": true } ]"`
- type: `JSON_TYPE_OBJECT_END,` name: `NULL`, path: `""`, value: `"{ \"foo\": 123, \"bar\": [ 1, 2, { \"baz\": true } ] }"`

If top-level element is an array: `[1, {"foo": 2}]`
- type: `JSON_TYPE_ARRAY_START`, name: `NULL`, path: `""`, value: `NULL`
- type: `JSON_TYPE_NUMBER`, name: `"0"`, path: `"[0]"`, value: `"1"`
- type: `JSON_TYPE_OBJECT_START`, name: `"1"`, path: `"[1]"`, value: `NULL`
- type: `JSON_TYPE_NUMBER`, name: `"foo"`, path: `"[1].foo"`, value: `"2"`
- type: `JSON_TYPE_OBJECT_END`, name: `NULL`, path: `"[1]"`, value: `"{\"foo\": 2}"`
- type: `JSON_TYPE_ARRAY_END`, name: `NULL`, path: `""`, value: `"[1, {"foo": 2}]"`

If top-level element is a scalar: `true`
- type: `JSON_TYPE_TRUE`, name: `NULL`, path: `""`, value: `"true"`
