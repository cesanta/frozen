JSON parser and emitter for C/C++
=================================

# Features

   * Portable to any environment
   * Simple, easy to understand API
   * Very small footprint
   * No dependencies
   * Code is strict ISO C and strict ISO C++ at the same time
   * Specialized for embedded use case: prints and scans directly to/from
     C/C++ variables
   * Parser provides low-level callback API and high-level scanf-like API
   * Supports superset of JSON: allows non-quoted identifiers as object keys
   * Complete 100% test coverage

# Parsing Usage Example

```
  // str has the following JSON string (notice keys are out of order):
  // { "a": 123, "d": true, "b": [1, 2], "c": "hi" }

  int a, b;
  char *c;
  json_scanf(str, strlen(str), "{ a:%d, b:%M, c:%Q, d:%B }",
             &a, &b, &c, scan_array, my_data);


  // This function is called by json_scanf() call above.
  // str is "[1, 2]", user_data is my_data.
  static void scan_array(const char *str, int len, void *user_data) {
    struct json_token t;
    int i;
    printf("Parsing array: %.*s\n", len, str);
    for (i = 0; json_scanf_array_elem(str, len, ".x", i, &t) > 0; i++) {
      printf("Index %d, token [%.*s]\n", i, t.len, t.ptr);
    }
  }
```

# Printing Usage Example

Note keys are not escaped. `json_printf()` escapes them.

```
  json_printf(&out, "{%Q: %d, x: [%B, %B], y: %Q}", "foo", 123, 0, -1, "hi");
  // Result:
  // {"foo": 123, "x": [false, true], "y": "hi"}
```

To print a complex object (for example, serialize a structure into an object),
use `%M` format specifier:

```
  struct my_struct { int a, b; } mys = {1,2};
  json_printf(&out, "{foo: %M, bar: %d}", print_my_struct, &mys, 3);
  // Result:
  // {"foo": {"a": 1, "b": 2}, "bar": 3}
```

```
int print_my_struct(struct json_out *out, va_list *ap) {
  struct my_struct *p = va_arg(*ap, struct my_struct *);
  return json_printf(out, "{a: %d, b: %d}", p->a, p->b);
}
```

# Low-level, callback based parsing API

`json_parse()` calls given callback function for each scanned value.
Callback receives path to the value, a JSON token that points to the value,
and arbitrary user data pointer.

The path is constructed using this rule:
- Root element has "" (empty string) path
- When an object starts, `.` (dot) is appended to the path
- When an object key is parsed, a key name is appended to the path
- When an array is parsed, for each element a `[ELEM_INDEX]` is appended

For example, consider the following json string:
`{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`.
The sequence of callback invocations will be as follows:
- path: `.foo`, token: `123`
- path: `.bar[0]`, token: `1`
- path: `.bar[1]`, token: `2`
- path: `.bar[2].baz`, token: `true`
- path: `.bar[2]`, token: `{ "baz": true }`
- path: `.bar`, token: `[ 1, 2, { "baz": true } ]`
- path: ` ` (empty string), token: `{ "foo": 123, "bar": [ 1, 2, { "baz": true } ] }`

If top-level element is an array: `[1, {"foo": 2}]`
- path: `[0]`, token: `1`
- path: `[1].foo`, token: `2`
- path: `[1]`, token: `{"foo": 2}`
- path: ` ` (empty string), token: `[1, {"foo": 2}]`

If top-level element is an scalar: `true`
- path: ` ` (empty string), token: `true`


```
/* Callback-based API */
typedef void (*json_parse_callback_t)(void *callback_data, const char *path,
                                      const struct json_token *token);

/*
 * Parse `json_string`, invoking `callback` function for each JSON token.
 * Return number of bytes processed
 */
int json_parse(const char *json_string, int json_string_length,
               json_parse_callback_t callback, void *callback_data);
```

# High level scanf-like parsing API

```
/*
 * Scan JSON string `str`, performing scanf-like conversions according to `fmt`.
 * `fmt` uses `scanf()`-like format, with the following differences:
 *
 * 1. Object keys in the format string don't have to be quoted, e.g. "{key: %d}"
 * 2. Order of keys in an object does not matter.
 * 3. Several extra format specifiers are supported:
 *    - %B: consumes `int *`, expects boolean `true` or `false`.
 *    - %Q: consumes `char **`, expects quoted, JSON-encoded string. Scanned
 *       string is malloc-ed, caller must free() the string. Scanned string
 *       is a JSON decoded, unescaped UTF-8 string.
 *    - %M: consumes custom scanning function pointer and
 *       `void *user_data` parameter - see json_scanner_t definition.
 *
 * Return number of elements successfully scanned & converted.
 * Negative number means scan error.
 */
int json_scanf(const char *str, int str_len, const char *fmt, ...);
int json_vscanf(const char *str, int str_len, const char *fmt, va_list ap);

/* json_scanf's %M handler  */
typedef void (*json_scanner_t)(const char *str, int len, void *user_data);

/*
 * Helper function to scan array item with given path and index.
 * Fills `token` with the matched JSON token.
 * Return 0 if no array element found, otherwise non-0.
 */
int json_scanf_array_elem(const char *s, int len, const char *path, int index,
                          struct json_token *token);
```

# Printing API

Frozen printing API is pluggable. Out of the box, Frozen provides a way
to print to a string buffer or to an opened file stream. It is easy to
to tell Frozen to print to other destination - for example, to a socket, etc.
Frozen does it by defining an "output context" descriptor, which has
a pointer to low-level printing function. If you want to print to some other
destination, just define your specific printing function and initialize
output context with it.

This is the definition of output context descriptor:

```
struct json_out {
  int (*printer)(struct json_out *, const char *str, size_t len);
  union {
    struct {
      char *buf;
      size_t size;
      size_t len;
    } buf;
    void *data;
    FILE *fp;
  } u;
};
```

Frozen provides two helper macros to initialize two builtin output
descriptors:

```
struct json_out out1 = JSON_OUT_BUF(buf, len);
struct json_out out2 = JSON_OUT_FILE(fp);
```

```
typedef int (*json_printf_callback_t)(struct json_out *, va_list *ap);

/*
 * Generate formatted output into a given sting buffer.
 * String values get escaped when printed (see `%M` specifier).
 * This is a superset of printf() function, with extra format specifiers:
 *  - `%B` print json boolean, `true` or `false`. Accepts an `int`.
 *  - `%Q` print quoted escaped string or `null`. Accepts a `const char *`.
 *  - `%M` invokes a json_printf_callback_t function. That callback function
 *  can consume more parameters.
 *
 * json_printf() also auto-escapes keys.
 *
 * Return number of bytes printed. If the return value is bigger then the
 * supplied buffer, that is an indicator of overflow. In the overflow case,
 * overflown bytes are not printed.
 */
int json_printf(struct json_out *, const char *fmt, ...);
int json_vprintf(struct json_out *, const char *fmt, va_list ap);

/*
 * Helper %M callback that prints contiguous C arrays.
 * Consumes void *array_ptr, size_t array_size, size_t elem_size, char *fmt
 * Return number of bytes printed.
 */
int json_printf_array(struct json_out *, va_list *ap);

```


# Contributions

To submit contributions, sign
[Cesanta CLA](https://docs.cesanta.com/contributors_la.shtml)
and send GitHub pull request. You retain the copyright on your contributions.

# Licensing

Frozen is released under commercial and [GNU GPL v.2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) open source licenses.

Commercial Projects:
Once your project becomes commercialised GPLv2 licensing dictates that you need to either open your source fully or purchase a commercial license. Cesanta offer full, royalty-free commercial licenses without any GPL restrictions. If your needs require a custom license, we’d be happy to work on a solution with you. [Contact us for pricing.] (https://www.cesanta.com/contact)

Prototyping:
While your project is still in prototyping stage and not for sale, you can use Frozen’s open source code without license restrictions.
