---
title: "API documentation"
---

```c
int parse_json(const char *json_string, int json_string_length,
               struct json_token *tokens_array, int size_of_tokens_array);
struct json_token *parse_json2(const char *json_string, int string_length);
```

`parse_json()` and `parse_json2()` parse JSON string.
`parse_json()` needs pre-allocated tokens array or NULL, whereas
`parse_json2()` allocates tokens array automatically.


`parse_json()` tokenizes `json_string` of length `json_string_length`.
If `tokens_array` is not `NULL`, then `parse_json()` will store tokens
in the `tokens_array`. Token with type
`JSON_TYPE_EOF` marks the end of parsed tokens. JSON token is defined as:

```c
struct json_token {
  const char *ptr;    // Points to the beginning of the token
  int len;            // Token length
  int num_desc;       // For arrays and object, total number of descendants
  int type;           // Type of the token, possible values below

#define JSON_TYPE_EOF     0   // Not a real token, but end-of-tokens marker
#define JSON_TYPE_STRING  1
#define JSON_TYPE_NUMBER  2
#define JSON_TYPE_OBJECT  3
#define JSON_TYPE_TRUE    4
#define JSON_TYPE_FALSE   5
#define JSON_TYPE_NULL    6
#define JSON_TYPE_ARRAY   7
};
```

If `tokens_array` is `NULL`, then `parse_json` just checks the validity of the
JSON string, and points where parsing stops. If `tokens_array` is not `NULL`,
it must be pre-allocated by the caller. Note that `struct json_token` just
points to the data inside `json_string`, it does not own the data. Thus the
token's lifetime is identical to the lifetime of `json_string`, until
`json_string` is freed or mutated. Return: On success, an offset inside
`json_string` is returned where parsing has finished. On failure, a negative
number is returned, one of:

```c
#define JSON_STRING_INVALID           -1
#define JSON_STRING_INCOMPLETE        -2
#define JSON_TOKEN_ARRAY_TOO_SMALL    -3
```

`parse_json2()` returns NULL on error and non-NULL on success.

Below is an illustration on how JSON string gets tokenized:

```
JSON string:      {  "key_1" : "value_1",  "key_2": [ 12345, null  ]   }

JSON_TYPE_OBJECT  |<-------------------------------------------------->|
JSON_TYPE_STRING      |<->|
JSON_TYPE_STRING                |<--->|
JSON_TYPE_STRING                            |<->|
JSON_TYPE_ARRAY                                     |<------------>|
JSON_TYPE_NUMBER                                      |<->|
JSON_TYPE_NULL                                               |<>|
JSON_TYPE_EOF
```

<!-- -->

```c
    const struct json_token *find_json_token(const struct json_token *toks,
                                             const char *path);
```

This is a convenience function to fetch specific values from the parsed
string. `toks` must be a valid array, successfully populated by `parse_json()`.
A `path` is a string, an accessor to the desired element of the JSON object,
as if it was written in Javascript. For example, if parsed JSON string is  
`"{ foo : { bar: [1, 2, 3] } }"`, then path `"foo.bar[0]"` would return a token
that points to number `"1"`.  
Return: pointer to the found token, or NULL on failure.

```c
struct json_out out = JSON_OUT_BUF(buf, len);
struct json_out out = JSON_OUT_FILE(fp);

typedef int (*json_printf_callback_t)(struct json_out *, va_list *ap);

int json_printf(struct json_out *, const char *fmt, ...);
int json_vprintf(struct json_out *, const char *fmt, va_list ap);
```
Generate formatted output into a given sting buffer.
This is a superset of printf() function, with extra format specifiers:

- `%B` print json boolean, `true` or `false`. Accepts an `int`.
- `%I` print int64_t value. Accepts an `int64_t`.
- `%Q` print quoted escaped string or `null`. Accepts a `const char *`.
- `%M` invokes a json_printf_callback_t function. That callback function
 can consume more parameters.

Return number of bytes printed.
