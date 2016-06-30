/*
 * Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013 Cesanta Software Limited
 * All rights reserved
 *
 * This library is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http: *www.gnu.org/licenses/>.
 *
 * You are free to use this library under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this library under a commercial
 * license, as set out in <http://cesanta.com/products.html>.
 */

#ifndef CS_FROZEN_FROZEN_H_
#define CS_FROZEN_FROZEN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

enum json_type {
  JSON_TYPE_EOF = 0, /* End of parsed tokens marker */
  JSON_TYPE_STRING = 1,
  JSON_TYPE_NUMBER = 2,
  JSON_TYPE_OBJECT = 3,
  JSON_TYPE_TRUE = 4,
  JSON_TYPE_FALSE = 5,
  JSON_TYPE_NULL = 6,
  JSON_TYPE_ARRAY = 7
};

struct json_token {
  const char *ptr;     /* Points to the beginning of the token */
  int len;             /* Token length */
  int num_desc;        /* For arrays and object, total number of descendants */
  enum json_type type; /* Type of the token, possible values above */
};

/* Error codes */
#define JSON_STRING_INVALID -1
#define JSON_STRING_INCOMPLETE -2
#define JSON_TOKEN_ARRAY_TOO_SMALL -3

int parse_json(const char *json_string, int json_string_length,
               struct json_token *tokens_array, int size_of_tokens_array);
struct json_token *parse_json2(const char *json_string, int string_length);
struct json_token *find_json_token(struct json_token *toks, const char *path);

/* Callback-based API */
typedef void (*json_parse_callback_t)(void *callback_data, const char *path,
                                      const struct json_token *token);

/*
 * Parse `json_string`, invoking `callback` function for each JSON token.
 * Return number of bytes processed
 */
int json_parse(const char *json_string, int json_string_length,
               json_parse_callback_t callback, void *callback_data);

/*
 * JSON generation API.
 * struct json_out abstracts output, allowing alternative printing plugins.
 */
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

extern int json_printer_buf(struct json_out *, const char *, size_t);
extern int json_printer_file(struct json_out *, const char *, size_t);

#define JSON_OUT_BUF(buf, len) \
  {                            \
    json_printer_buf, {        \
      { buf, len, 0 }          \
    }                          \
  }
#define JSON_OUT_FILE(fp)   \
  {                         \
    json_printer_file, {    \
      { (void *) fp, 0, 0 } \
    }                       \
  }

typedef int (*json_printf_callback_t)(struct json_out *, va_list *ap);

/*
 * Generate formatted output into a given sting buffer.
 * This is a superset of printf() function, with extra format specifiers:
 *  - `%B` print json boolean, `true` or `false`. Accepts an `int`.
 *  - `%Q` print quoted escaped string or `null`. Accepts a `const char *`.
 *  - `%M` invokes a json_printf_callback_t function. That callback function
 *  can consume more parameters.
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_FROZEN_FROZEN_H_ */
