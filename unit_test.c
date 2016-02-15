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

/*
 * To unit test on Mac system, do
 *
 * g++ unit_test.c -o unit_test -W -Wall -g -O0 -fprofile-arcs -ftest-coverage
 * clang unit_test.c -o unit_test -W -Wall -g -O0 -fprofile-arcs -ftest-coverage
 * ./unit_test
 * gcov -a unit_test.c
 */

#include "frozen.c"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FAIL(str, line)                           \
  do {                                            \
    printf("Fail on line %d: [%s]\n", line, str); \
    return str;                                   \
  } while (0)

#define ASSERT(expr)                    \
  do {                                  \
    static_num_tests++;                 \
    if (!(expr)) FAIL(#expr, __LINE__); \
  } while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define RUN_TEST(test)        \
  do {                        \
    const char *msg = test(); \
    if (msg) return msg;      \
  } while (0)

static int static_num_tests = 0;

static int cmp_token(const struct json_token *tok, const char *str,
                     enum json_type type) {
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
      "1", "a:3", "\x01", "{:", " { 1", "{a:\"\n\"}", "{a:1x}", "{a:1e}",
      "{a:.1}", "{a:0.}", "{a:0.e}", "{a:0.e1}", "{a:0.1e}", "{a:\"\\u\" } ",
      "{a:\"\\yx\"}", "{a:\"\\u111r\"}", NULL};
  static const char *incomplete_tests[] = {
      "", " \r\n\t", "{", " { a", "{a:", "{a:\"", " { a : \"xx", "{a:12",
      "{a:\"\\uf", "{a:\"\\uff", "{a:\"\\ufff", "{a:\"\\uffff",
      "{a:\"\\uffff\"", "{a:\"\\uffff\" ,", "{a:n", "{a:nu", "{a:nul",
      "{a:null", NULL};
  static const struct {
    const char *str;
    int expected_len;
  } success_tests[] = {{"{}", 2},
                       /* 2, 3, 4 byte utf-8 chars */
                       {"{a:\"\xd0\xb1\xe3\x81\xaf\xf0\xa2\xb3\x82\"}", 15},
                       {"{a:\"\\u0006\"}", 12},
                       {" { } ", 4},
                       {"{a:1}", 5},
                       {"{a:1.23}", 8},
                       {"{a:1e23}", 8},
                       {"{a:1.23e2}", 10},
                       {"{a:-123}", 8},
                       {"{a:-1.3}", 8},
                       {"{a:-1.3e-2}", 11},
                       {"{a:\"\"}", 6},
                       {"{a:\" \\n\\t\\r\"}", 13},
                       {" {a:[1]} 123456", 8},
                       {" {a:[]} 123456", 7},
                       {" {a:[1,2]} 123456", 10},
                       {"{a:1,b:2} xxxx", 9},
                       {"{a:1,b:{},c:[{}]} xxxx", 17},
                       {"{a:true,b:[false,null]} xxxx", 23},
                       {"[1.23, 3, 5]", 12},
                       {"[13, {\"a\":\"hi there\"}, 5]", 25},
                       {NULL, 0}};
  const char *s1 =
      " { a: 1, b: \"hi there\", c: true, d: false, "
      " e : null, f: [ 1, -2, 3], g: { \"1\": [], h: [ 7 ] } } ";
  const char *s2 =
      "{ a: 1, b: \"hi there\", c: true, d: false, "
      " e : null, f: [ 1, -2, 3], g: { \"1\": [], h: [ 7 ] } }";
  const char *s3 = "{ \"1\": [], h: [ 7 ] }";
  int i;

  ASSERT(parse_json(NULL, 0, NULL, 0) == JSON_STRING_INVALID);
  for (i = 0; invalid_tests[i] != NULL; i++) {
    ASSERT(parse_json(invalid_tests[i], strlen(invalid_tests[i]), ar, size) ==
           JSON_STRING_INVALID);
  }

  for (i = 0; incomplete_tests[i] != NULL; i++) {
    ASSERT(parse_json(incomplete_tests[i], strlen(incomplete_tests[i]), ar,
                      size) == JSON_STRING_INCOMPLETE);
  }

  for (i = 0; success_tests[i].str != NULL; i++) {
    ASSERT(parse_json(success_tests[i].str, strlen(success_tests[i].str), ar,
                      size) == success_tests[i].expected_len);
  }

  ASSERT(parse_json("{}", 2, ar, 1) == JSON_TOKEN_ARRAY_TOO_SMALL);
  ASSERT(parse_json("{}", 2, ar, 2) == 2);
  ASSERT(cmp_token(&ar[0], "{}", JSON_TYPE_OBJECT));
  ASSERT(ar[1].type == JSON_TYPE_EOF);

  ASSERT(parse_json(s1, strlen(s1), NULL, 0) > 0);
  ASSERT(parse_json(s1, strlen(s1), ar, 10) == JSON_TOKEN_ARRAY_TOO_SMALL);
  ASSERT(parse_json(s1, strlen(s1), ar, size) > 0);
  ASSERT(cmp_token(&ar[0], s2, JSON_TYPE_OBJECT));
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
  ASSERT(cmp_token(&ar[17], s3, JSON_TYPE_OBJECT));
  ASSERT(cmp_token(&ar[18], "1", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[19], "[]", JSON_TYPE_ARRAY));
  ASSERT(cmp_token(&ar[20], "h", JSON_TYPE_STRING));
  ASSERT(cmp_token(&ar[21], "[ 7 ]", JSON_TYPE_ARRAY));
  ASSERT(cmp_token(&ar[22], "7", JSON_TYPE_NUMBER));
  ASSERT(ar[23].type == JSON_TYPE_EOF);

  ASSERT(find_json_token(ar, "a") == &ar[2]);
  ASSERT(find_json_token(ar, "f") == &ar[12]);
  ASSERT(find_json_token(ar, "g.h") == &ar[21]);
  ASSERT(find_json_token(ar, "g.h[0]") == &ar[22]);
  ASSERT(find_json_token(ar, "g.h[1]") == NULL);
  ASSERT(find_json_token(ar, "g.h1") == NULL);
  ASSERT(find_json_token(ar, "") == NULL);
  ASSERT(find_json_token(ar, NULL) == NULL);

  return NULL;
}

static const char *test_config(void) {
  static const char *config_str = "{ ports: [ 80, 443 ] } ";
  struct json_token tokens[100];
  int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

  ASSERT(parse_json(config_str, strlen(config_str), tokens, tokens_size) > 0);
  ASSERT(tokens[0].type == JSON_TYPE_OBJECT);
  ASSERT(tokens[1].type == JSON_TYPE_STRING);
  ASSERT(tokens[2].type == JSON_TYPE_ARRAY);
  ASSERT(tokens[3].type == JSON_TYPE_NUMBER);
  ASSERT(tokens[4].type == JSON_TYPE_NUMBER);
  ASSERT(tokens[5].type == JSON_TYPE_EOF);

  ASSERT(find_json_token(tokens, "ports") == &tokens[2]);
  ASSERT(find_json_token(tokens, "ports[0]") == &tokens[3]);
  ASSERT(find_json_token(tokens, "ports[1]") == &tokens[4]);
  ASSERT(find_json_token(tokens, "ports[3]") == NULL);
  ASSERT(find_json_token(tokens, "foo.bar") == NULL);

  return NULL;
}

static const char *test_emit_overflow(void) {
  char buf[1000];

  memset(buf, 0, sizeof(buf));
  ASSERT(json_emit_unquoted_str(buf, 0, "hi", 2) == 2);
  ASSERT(json_emit_quoted_str(buf, 0, "hi", 2) == 4);
  ASSERT(buf[0] == '\0');

  return NULL;
}

static const char *test_emit_escapes(void) {
  const char *s4 = "\"\\\"\\\\\\b\\f\\n\\r\\t\"";
  char buf[1000];
  ASSERT(json_emit_quoted_str(buf, sizeof(buf), "\"\\\b\f\n\r\t", 7) > 0);
  ASSERT(strcmp(buf, s4) == 0);
  return NULL;
}

static const char *test_emit(void) {
  char buf[1000], *p = buf;
  const char *s5 = "{\"foo\":[-123,1.23,true]}";
  const char *s6 = "{\"foo\":[-7,true, false,null]}";

  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, "{", 1);
  p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "foo", 3);
  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, ":[", 2);
  p += json_emit_long(p, &buf[sizeof(buf)] - p, -123);
  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, ",", 1);
  p += json_emit_double(p, &buf[sizeof(buf)] - p, 1.23);
  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, ",", 1);
  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, "true", 4);
  p += json_emit_unquoted_str(p, &buf[sizeof(buf)] - p, "]}", 2);

  ASSERT(strcmp(buf, s5) == 0);
  ASSERT(p < &buf[sizeof(buf)]);

  ASSERT(json_emit(buf, sizeof(buf), "{v:[i,f,V]}", "foo", 3, (long) -123, 1.23,
                   "true", 4) > 0);
  ASSERT(strcmp(buf, s5) == 0);

  ASSERT(json_emit(buf, 4, "{S:i}", "a", 12345) > 4);
  ASSERT(json_emit(buf, sizeof(buf), "{S:d}", "a", 12345) == 0);

  ASSERT(json_emit(buf, sizeof(buf), "{s:[i,T, F,N]}", "foo", (long) -7) > 0);
  ASSERT(strcmp(buf, s6) == 0);

  return NULL;
}

static const char *test_nested(void) {
  struct json_token ar[100];
  const char *s = "{ a : [ [1, 2, { b : 2 } ] ] }";
  enum json_type types[] = {
      JSON_TYPE_OBJECT, JSON_TYPE_STRING, JSON_TYPE_ARRAY,  JSON_TYPE_ARRAY,
      JSON_TYPE_NUMBER, JSON_TYPE_NUMBER, JSON_TYPE_OBJECT, JSON_TYPE_STRING,
      JSON_TYPE_NUMBER, JSON_TYPE_EOF};
  int i, ar_size = ARRAY_SIZE(ar), types_size = ARRAY_SIZE(types);

  ASSERT(parse_json(s, strlen(s), ar, ar_size) == (int) strlen(s));
  for (i = 0; i < types_size; i++) {
    ASSERT(ar[i].type == types[i]);
  }
  ASSERT(find_json_token(ar, "a[0]") == &ar[3]);
  ASSERT(find_json_token(ar, "a[0][0]") == &ar[4]);
  ASSERT(find_json_token(ar, "a[0][1]") == &ar[5]);
  ASSERT(find_json_token(ar, "a[0][2]") == &ar[6]);
  ASSERT(find_json_token(ar, "a[0][2].b") == &ar[8]);

  return NULL;
}

static const char *test_realloc(void) {
  struct json_token *p;
  ASSERT(parse_json2("{ foo: 2 }", 2) == NULL);
  ASSERT((p = parse_json2("{ foo: 2 }", 10)) != NULL);
  free(p);
  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_errors);
  RUN_TEST(test_config);
  RUN_TEST(test_emit);
  RUN_TEST(test_emit_escapes);
  RUN_TEST(test_emit_overflow);
  RUN_TEST(test_nested);
  RUN_TEST(test_realloc);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
