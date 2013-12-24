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

#if 0
static int cmp_token(const struct json_token *tok, const char *str) {
  return (int) strlen(str) == tok->len && memcmp(tok->ptr, str, tok->len) == 0;
}
#endif

static const char *test_errors(void) {
  struct json_token ar[100];
  int size = ARRAY_SIZE(ar);
  static const char *invalid_tests[] = {
    "1", "a:3", "\x01", "{:", " { 1",
    NULL
  };
  static const char *incomplete_tests[] = {
    "", " \r\n\t", "{", " { a", "{a:", "{a:\"", " { a : \"xx",
    NULL
  };
  static const struct { const char *str; int expected_len; } success_tests[] = {
    { NULL, 0 }
  };
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
