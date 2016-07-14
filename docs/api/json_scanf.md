---
title: json_scanf()
---

```c
int json_scanf(const char *str, int str_len, const char *fmt, ...);
int json_vscanf(const char *str, int str_len, const char *fmt, va_list ap);

/* json_scanf's %M handler  */
typedef void (*json_scanner_t)(const char *str, int len, void *user_data);

```

Scan JSON string `str`, performing scanf-like conversions according to `fmt`.
`fmt` uses `scanf()`-like format, with the following differences:

1. Object keys in the format string don't have to be quoted, e.g. "{key: %d}"
2. Order of keys in a format string does not matter, and format string may
   omit keys to fetch only those that are of interest, for example,
   assume `str` is a JSON string `{ "a": 123, "b": "hi", c: true }`.
   We can fetch only the value of the `c` key:
      ```C
      int value = 0;
      json_scanf(str, strlen(str), "{c: %B}", &value);
      ```
3. Several extra format specifiers are supported:
   - %B: consumes `int *`, expects boolean `true` or `false`.
   - %Q: consumes `char **`, expects quoted, JSON-encoded string. Scanned
      string is malloc-ed, caller must free() the string. Scanned string
      is a JSON decoded, unescaped UTF-8 string.
   - %M: consumes custom scanning function pointer and
      `void *user_data` parameter - see json_scanner_t definition.

Return number of elements successfully scanned & converted.
Negative number means scan error.


```c
int json_scanf_array_elem(const char *s, int len,
                          const char *path,
                          int index,
                          struct json_token *token);
```

A helper function to scan array item with given path and index.
Fills `token` with the matched JSON token.
Return 0 if no array element found, otherwise non-0.
