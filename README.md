JSON parser and generator for C/C++
===========================================

# Features

   * Portable to any environment
   * Simple API: 2 functions for parsing and 4 for generation
   * Very small footprint
   * No dependencies
   * Makes no memory allocations
   * Code is strict ISO C and strict ISO C++ at the same time
   * Supports superset of JSON: allows non-quoted identifiers as object keys
   * Complete 100% test coverage

# How to use it

   1. Copy `frozen.c` and `frozen.h` to your project
   2. Add `frozen.c` to the list of source files
   3. Parsing with Frozen is done in two steps: first, split JSON string
      into tokens by `parse_json()`. Second, search for certain
      parameters in parsed string by `find_json_token()`. Below is an example,
      error handling is omitted for clarity:


        #include <stdio.h>
        #include "frozen.h"

        int main(void) {
          static const char *json = " { foo: 1, bar: 2 } ";
          struct json_token arr[10], *tok;

          // Tokenize json string, fill in tokens array
          parse_json(json, strlen(json), arr, sizeof(arr) / sizeof(arr[0]));

          // Search for parameter "bar" and print it's value
          tok = find_json_token(arr, "bar");
          printf("Value of bar is: [%.*s]\n", tok->len, tok->ptr);

          return 0;
        }

# API documentation

    int parse_json(const char *json_string, int json_string_length,
                   struct json_token *tokens_array, int size_of_tokens_array);

Tokenizes `json_string` of length `json_string_length`.
If `tokens_array` is not `NULL`, then
all `parse_json` will store tokens in the `tokens_array`. Token with type
`JSON_TYPE_EOF` marks the end of parsed tokens. JSON token is defined as:

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

If `tokens_array` is `NULL`, then `parse_json` just checks the validity of
the JSON string, and points where parsing stops. If `tokens_array` is not
`NULL`, it must be pre-allocated by the caller. Note that `struct json_token`
just points to the data inside `json_string`, it does not own the data. Thus
the token's lifetime is identical to the lifetime of `json_string`, until
`json_string` is freed or mutated.  
Return: On success, an offset inside `json_string` is returned
where parsing has finished. On failure, a negative number is
returned, one of:

    #define JSON_STRING_INVALID           -1
    #define JSON_STRING_INCOMPLETE        -2
    #define JSON_TOKEN_ARRAY_TOO_SMALL    -3

Below is an illustration on how JSON string gets tokenized:

       JSON string:      {  "key_1" : "value_1",  "key_2": [ 12345, null  ]   }

       JSON_TYPE_OBJECT  |<-------------------------------------------------->|
       JSON_TYPE_STRING      |<->|
       JSON_TYPE_STRING                |<--->|
       JSON_TYPE_STRING                            |<->|
       JSON_TYPE_ARRAY                                     |<------------>|
       JSON_TYPE_NUMBER                                      |<->|
       JSON_TYPE_NULL                                               |<>|
       JSON_TYPE_EOF

<!-- -->

    const struct json_token *find_json_token(const struct json_token *toks,
                                             const char *path);

This is a convenience function to fetch specific values from the parsed
string. `toks` must be a valid array, successfully populated by `parse_json()`.
A `path` is a string, an accessor to the desired element of the JSON object,
as if it was written in Javascript. For example, if parsed JSON string is  
`"{ foo : { bar: [1, 2, 3] } }"`, then path `"foo.bar[0]"` would return a token
that points to number `"1"`.  
Return: pointer to the found token, or NULL on failure.


    int json_emit_int(char *buf, int buf_len, long int value);
    int json_emit_double(char *buf, int buf_len, double value);
    int json_emit_quoted_str(char *buf, int buf_len, const char *str);
    int json_emit_raw_str(char *buf, int buf_len, const char *str);

These functions are used to generate JSON string. All of them accept
a destination buffer and a value to output, and return number of bytes printed.
Returned value can be bigger then destination buffer size, this is an
indication of overflow. If there is no overflow, a buffer is guaranteed to
be nul-terminated. Numbers are printed by `json_emit_double()` and
`json_emit_int()` functions, strings are printed by `json_emit_quoted_str()`
function. Values for `null`, `true`, `false`, and characters
`{`, `}`, `[`, `]`, `,`, `:` are printed by
`json_emit_raw_str()` function.

## Example: accessing configuration parameters

    #include "frozen.h"

    static const char *config_str = " { ports: [ 80, 443 ] } ";
    struct json_token tokens[10];
    int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

    // Parse config string and make sure tokenization is correct
    ASSERT(parse_json(config_str, strlen(config_str), tokens, tokens_size) > 0);

    ASSERT(tokens[0].type == JSON_TYPE_OBJECT);   // Tokens are populated
    ASSERT(tokens[1].type == JSON_TYPE_STRING);   // in order of their
    ASSERT(tokens[2].type == JSON_TYPE_ARRAY);    // appearance in the
    ASSERT(tokens[3].type == JSON_TYPE_NUMBER);   // JSON string
    ASSERT(tokens[4].type == JSON_TYPE_NUMBER);
    ASSERT(tokens[5].type == JSON_TYPE_EOF);      // Last token is always EOF

    // Fetch port values
    ASSERT(find_json_token(tokens, "ports") == &tokens[2]);
    ASSERT(find_json_token(tokens, "ports[0]") == &tokens[3]);
    ASSERT(find_json_token(tokens, "ports[1]") == &tokens[4]);
    ASSERT(find_json_token(tokens, "ports[3]") == NULL);  // Outside boundaries
    ASSERT(find_json_token(tokens, "foo.bar") == NULL);   // Nonexistent

## Example: generating JSON string `{"foo":[-123,true]}`

    char buf[1000], *p = buf;

    p += json_emit_raw_str(p, &buf[sizeof(buf)] - p, "{");
    p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "foo");
    p += json_emit_raw_str(p, &buf[sizeof(buf)] - p, ":[");
    p += json_emit_int(p, &buf[sizeof(buf)] - p, -123);
    p += json_emit_raw_str(p, &buf[sizeof(buf)] - p, ",true]}");

    ASSERT(strcmp(buf, "{\"foo\":[-123,true]}") == 0);
    ASSERT(p < &buf[sizeof(buf)]);

# License

Frozen is released under
[GNU GPL v.2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
Businesses have an option to get non-restrictive, royalty-free commercial
license and professional support from
[Cesanta Software](http://cesanta.com).

[Super Light DNS Resolver](https://github.com/cesanta/sldr),
[Super Light Regexp Library](https://github.com/cesanta/slre),
[Mongoose web server](https://github.com/cesanta/mongoose)
are other projects by Cesanta Software, developed with the same philosophy
of functionality and simplicity.
