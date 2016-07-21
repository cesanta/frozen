---
title: json_walk()
---

```c
/* Callback-based API */
typedef void (*json_walk_callback_t)(void *callback_data, const char *path,
                                     const struct json_token *token);

/*
 * Parse `json_string`, invoking `callback` function for each JSON token.
 * Return number of bytes processed
 */
int json_walk(const char *json_string, int json_string_length,
              json_walk_callback_t callback, void *callback_data);
```

`json_walk()` is a low-level, callback based parsing API.
`json_walk()` calls given callback function for each scanned value.

Callback receives a path to the value, a JSON token that points to the value,
and arbitrary user data pointer.

The path is constructed using this rule:
- Root element has "" (empty string) path
- When an object starts, `.` (dot) is appended to the path
- When an object key is parsed, a key name is appended to the path
- When an array is parsed, for each element an `[ELEMENT_INDEX]` is appended

For example, consider the following json string:
`{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`.
The sequence of callback invocations will be as follows:
- path: `.foo`, token: `123`
- path: `.bar[0]`, token: `1`
- path: `.bar[1]`, token: `2`
- path: `.bar[2].baz`, token: `true`
- path: `.bar[2]`, token: `{ "baz": true }`
- path: `.bar`, token: `[ 1, 2, { "baz": true } ]`
- path: "" (empty string), token: `{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`

If top-level element is an array: `[1, {"foo": 2}]`
- path: `[0]`, token: `1`
- path: `[1].foo`, token: `2`
- path: `[1]`, token: `{"foo": 2}`
- path: "" (empty string), token: `[1, {"foo": 2}]`

If top-level element is an scalar: `true`
- path: "" (empty string), token: `true`
