---
title: json_printf()
---

The Frozen printing API is pluggable. Out of the box, Frozen provides a way
to print to a string buffer or to an opened file stream. It is easy to
to tell Frozen to print to another destination, for example, to a socket, etc.
Frozen does this by defining an "output context" descriptor which has
a pointer to a low-level printing function. If you want to print to another
destination, just define your specific printing function and initialise
output context with it.

This is the definition of the output context descriptor:

```c
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

Frozen provides two helper macros to initialise two built-in output
descriptors:

```c
struct json_out out1 = JSON_OUT_BUF(buf, len);
struct json_out out2 = JSON_OUT_FILE(fp);
```

```c
typedef int (*json_printf_callback_t)(struct json_out *, va_list *ap);
int json_printf(struct json_out *, const char *fmt, ...);
int json_vprintf(struct json_out *, const char *fmt, va_list ap);
```

Generates formatted output into a given sting buffer.
String values escape when printed (see `%M` specifier).
This is a superset of the printf() function, with extra format specifiers:
- `%B` prints JSON boolean, `true` or `false`. Accepts an `int`.
- `%Q` prints quoted escaped string or `null`. Accepts a `const char *`.
- `%.*Q` like `%Q` but accepts the length of the string explicitly, pretty much like `%.*s`.
Embedded NUL bytes are supported and will be properly encoded as `\u0000`.
Accepts an `int` length and a `const char *`.
- `%M` invokes a json_printf_callback_t function. That callback function
can consume more parameters.

`json_printf()` also auto-escapes keys.

Returns the number of bytes printed. If the return value is bigger then the
supplied buffer, that is an indicator of overflow. In the overflow case,
overflown bytes are not printed.

```c
int json_printf_array(struct json_out *, va_list *ap);
```

A helper `%M` callback that prints contiguous C arrays.
Consumes `void *array_ptr, size_t array_size, size_t elem_size, char *fmt`
Returns number of bytes printed.
