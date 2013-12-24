// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013 Cesanta Software Limited
// All rights reserved
//
// This library is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this library under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#ifndef FROZEN_HEADER_INCLUDED
#define FROZEN_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct json_token {
  const char *ptr;    // Points to the beginning of the token
  int len;            // Token length
  int num_children;   // Number of children for array and objects
  int type;           // Type of the token, possible values below

#define JSON_TYPE_EOF     0   // End of parsed tokens marker
#define JSON_TYPE_STRING  1
#define JSON_TYPE_NUMERIC 2
#define JSON_TYPE_OBJECT  3
#define JSON_TYPE_TRUE    4
#define JSON_TYPE_FALSE   5
#define JSON_TYPE_NULL    6
};

// Error codes
#define JSON_STRING_INVALID           -1
#define JSON_STRING_INCOMPLETE        -2
#define JSON_TOKEN_ARRAY_TOO_SMALL    -3
#define JSON_OUTPUT_BUFFER_TOO_SMALL  -4

// Parse JSON string, store tokens in tokens_array. Last token has type
// JSON_TYPE_EOF. Return offset in json string where parsing ended,
// or negative error code on failure
int parse_json(const char *json_string, int json_string_length,
               struct json_token *tokens_array, int size_of_tokens_array);

int find_json_token(const struct json_token *toks, int num_toks, const char *p);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FROZEN_HEADER_INCLUDED
