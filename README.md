JSON parser for C/C++
===========================================

# Features

   * Portable to any environment
   * Simple API: one function for parsing and one helper function
     for fetching parsed values
   * Supports superset of JSON: allows non-quoted identifiers as object keys
   * Very small footprint
   * No dependencies
   * Makes no memory allocations
   * Code is strict ISO C and valid C++

# API

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

<!-- -->

    const struct json_token *find_json_token(const struct json_token *toks,
                                             const char *path);

This is a convenience function to fetch specific values from the parsed
string. `toks` must be a valid array, successfully populated by `parse_json()`.
A `path` is a string, an accessor to the desired element of the JSON object,
as if it was written in Javascript. For example, if parsed JSON string is
"{ foo : { bar: [1, 2, 3] } }", then path "foo.bar[0]" would return a token
that points to number "1".  
Return: pointer to the found token, or NULL on failure.


## Example: accessing configuration parameters

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
