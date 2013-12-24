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

#include "frozen.c"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FAIL(str, line) do {                    \
  printf("Fail on line %d: [%s]\n", line, str); \
  return str;                                   \
} while (0)

#define ASSERT(expr) do {                       \
  static_num_tests++;                           \
  if (!(expr)) FAIL(#expr, __LINE__);           \
} while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define RUN_TEST(test) do { const char *msg = test(); \
  if (msg) return msg; } while (0)

static int static_num_tests = 0;

static int cmp_token(const struct json_token *tok, const char *str, int type) {
#if 0
  printf("[%.*s] [%s]\n", tok->len, tok->ptr, str);
#endif
  return tok->type == type && (int) strlen(str) == tok->len &&
    memcmp(tok->ptr, str, tok->len) == 0;
}

static const char *test_errors(void) {
  struct json_token ar[100];
  int size = ARRAY_SIZE(ar);
  static const char *invalid_tests[] = {
    "1", "a:3", "\x01", "{:", " { 1", "{a:\"\n\"}",
    NULL
  };
  static const char *incomplete_tests[] = {
    "", " \r\n\t", "{", " { a", "{a:", "{a:\"", " { a : \"xx", "{a:12",
    NULL
  };
  static const struct { const char *str; int expected_len; } success_tests[] = {
    { "{}", 2 },
    { " { } ", 4 },
    { "{a:1}", 5 },
    { "{a:1}", 5 },
    { "{a:\"\"}", 6 },
    { "{a:\" \\n\\t\\r\"}", 13 },
    { " {a:[1]} 123456", 8 },
    { " {a:[]} 123456", 7 },
    { " {a:[1,2]} 123456", 10 },
    { "{a:1,b:2} xxxx", 9 },
    { "{a:1,b:{},c:[{}]} xxxx", 17 },
    { "{a:true,b:[false,null]} xxxx", 23 },
    { NULL, 0 }
  };
  const char *s1 = " { a: 1, b: \"hi there\", c: true, d: false, "
    " e : null, f: [ 1, -2, 3], g: { \"1\": [], h: {} } } ";
  int i;

  ASSERT(parse_json(NULL, 0, NULL, 0) == JSON_STRING_INVALID);
  for (i = 0; invalid_tests[i] != NULL; i++) {
    ASSERT(parse_json(invalid_tests[i], strlen(invalid_tests[i]),
                      ar, size) == JSON_STRING_INVALID);
  }

  for (i = 0; incomplete_tests[i] != NULL; i++) {
    ASSERT(parse_json(incomplete_tests[i], strlen(incomplete_tests[i]),
                      ar, size) == JSON_STRING_INCOMPLETE);
  }

  for (i = 0; success_tests[i].str != NULL; i++) {
    ASSERT(parse_json(success_tests[i].str, strlen(success_tests[i].str),
                      ar, size) == success_tests[i].expected_len);
  }

  ASSERT(parse_json("{}", 2, ar, 1) == JSON_TOKEN_ARRAY_TOO_SMALL);
  ASSERT(parse_json("{}", 2, ar, 2) == 2);
  ASSERT(cmp_token(&ar[0], "{}", JSON_TYPE_OBJECT));
  ASSERT(ar[1].type == JSON_TYPE_EOF);

  ASSERT(parse_json(s1, strlen(s1), NULL, 0) > 0);
  ASSERT(parse_json(s1, strlen(s1), ar, 10) == JSON_TOKEN_ARRAY_TOO_SMALL);
  ASSERT(parse_json(s1, strlen(s1), ar, size) > 0);
  ASSERT(cmp_token(&ar[0], "{ a: 1, b: \"hi there\", c: true, d: false, "
                   " e : null, f: [ 1, -2, 3], g: { \"1\": [], h: {} } }",
                   JSON_TYPE_OBJECT));
  ASSERT(cmp_token(&ar[1], "a", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[2], "1", JSON_TYPE_NUMBER));
  ASSERT(cmp_token(&ar[3], "b", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[4], "hi there", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[5], "c", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[6], "true", JSON_TYPE_TRUE));
  ASSERT(cmp_token(&ar[7], "d", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[8], "false", JSON_TYPE_FALSE));
  ASSERT(cmp_token(&ar[9], "e", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[10], "null", JSON_TYPE_NULL));
  ASSERT(cmp_token(&ar[11], "f", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[12], "[ 1, -2, 3]", JSON_TYPE_ARRAY));
  ASSERT(cmp_token(&ar[13], "1", JSON_TYPE_NUMBER));
  ASSERT(cmp_token(&ar[14], "-2", JSON_TYPE_NUMBER));
  ASSERT(cmp_token(&ar[15], "3", JSON_TYPE_NUMBER));
  ASSERT(cmp_token(&ar[16], "g", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[17], "{ \"1\": [], h: {} }" , JSON_TYPE_OBJECT));
  ASSERT(cmp_token(&ar[18], "1", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[19], "[]", JSON_TYPE_ARRAY));
  ASSERT(cmp_token(&ar[20], "h", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[21], "{}", JSON_TYPE_OBJECT));
  ASSERT(ar[22].type == JSON_TYPE_EOF);

  ASSERT(ar[0].num_children == 21);
  ASSERT(ar[1].num_children == 0);
  ASSERT(ar[2].num_children == 0);
  ASSERT(ar[3].num_children == 0);
  ASSERT(ar[4].num_children == 0);
  ASSERT(ar[5].num_children == 0);
  ASSERT(ar[6].num_children == 0);
  ASSERT(ar[7].num_children == 0);
  ASSERT(ar[8].num_children == 0);
  ASSERT(ar[9].num_children == 0);
  ASSERT(ar[10].num_children == 0);
  ASSERT(ar[11].num_children == 0);
  ASSERT(ar[12].num_children == 3);
  ASSERT(ar[13].num_children == 0);
  ASSERT(ar[14].num_children == 0);
  ASSERT(ar[15].num_children == 0);
  ASSERT(ar[16].num_children == 0);
  ASSERT(ar[17].num_children == 4);
  ASSERT(ar[18].num_children == 0);
  ASSERT(ar[19].num_children == 0);
  ASSERT(ar[20].num_children == 0);
  ASSERT(ar[21].num_children == 0);

  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_errors);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
