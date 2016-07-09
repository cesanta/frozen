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

#include <stdio.h>
#include <stdlib.h>
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

static const char *test_errors(void) {
  /* clang-format off */
  static const char *invalid_tests[] = {
      "p",        "a:3",           "\x01",         "{:",
      " { 1",     "{a:\"\n\"}",    "{a:1x}",       "{a:1e}",
      "{a:.1}",   "{a:0.}",        "{a:0.e}",      "{a:0.e1}",
      "{a:0.1e}", "{a:\"\\u\" } ", "{a:\"\\yx\"}", "{a:\"\\u111r\"}",
      NULL};
  static const char *incomplete_tests[] = {"",
                                           " \r\n\t",
                                           "{",
                                           " { a",
                                           "{a:",
                                           "{a:\"",
                                           " { a : \"xx",
                                           "{a:12",
                                           "{a:\"\\uf",
                                           "{a:\"\\uff",
                                           "{a:\"\\ufff",
                                           "{a:\"\\uffff",
                                           "{a:\"\\uffff\"",
                                           "{a:\"\\uffff\" ,",
                                           "{a:n",
                                           "{a:nu",
                                           "{a:nul",
                                           "{a:null",
                                           NULL};
  /* clang-format on */
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
  int i;

  ASSERT(json_parse(NULL, 0, NULL, 0) == JSON_STRING_INVALID);
  for (i = 0; invalid_tests[i] != NULL; i++) {
    ASSERT(json_parse(invalid_tests[i], strlen(invalid_tests[i]), NULL, NULL) ==
           JSON_STRING_INVALID);
  }

  for (i = 0; incomplete_tests[i] != NULL; i++) {
    ASSERT(json_parse(incomplete_tests[i], strlen(incomplete_tests[i]), NULL,
                      NULL) == JSON_STRING_INCOMPLETE);
  }

  for (i = 0; success_tests[i].str != NULL; i++) {
    ASSERT(json_parse(success_tests[i].str, strlen(success_tests[i].str), NULL,
                      NULL) == success_tests[i].expected_len);
  }

  ASSERT(json_parse("{}", 2, NULL, NULL) == 2);
  ASSERT(json_parse(s1, strlen(s1), NULL, 0) > 0);

  return NULL;
}

struct my_struct {
  int a, b;
};

static int print_my_struct(struct json_out *out, va_list *ap) {
  struct my_struct *p = va_arg(*ap, struct my_struct *);
  return json_printf(out, "{a: %d, b: %d}", p->a, p->b);
}

static const char *test_json_printf(void) {
  char buf[200] = "";

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "42";
    json_printf(&out, "%d", 42);
    ASSERT(strcmp(buf, result) == 0);
  }

  /* platform specific compatibility where it matters */
  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "16045690985373621933 42";
    json_printf(&out, "%" UINT64_FMT " %d", 0xdeadbeeffee1deadUL, 42);
    ASSERT(strcmp(buf, result) == 0);
  }
  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "12 42";
    size_t foo = 12;
    json_printf(&out, "%" SIZE_T_FMT " %d", foo, 42);
    ASSERT(strcmp(buf, result) == 0);
  }

  /* people live in the future today, %llu works even on recent windozes */
  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "16045690985373621933 42";
    json_printf(&out, "%llu %d", 0xdeadbeeffee1deadUL, 42);
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "12 42";
    size_t foo = 12;
    json_printf(&out, "%zu %d", foo, 42);
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "{\"foo\": 123, \"x\": [false, true], \"y\": \"hi\"}";
    json_printf(&out, "{%Q: %d, x: [%B, %B], y: %Q}", "foo", 123, 0, -1, "hi");
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    int arr[] = {-2387, 943478};
    json_printf(&out, "%M", json_printf_array, arr, sizeof(arr), sizeof(arr[0]),
                "%d");
    ASSERT(strcmp(buf, "[-2387, 943478]") == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    double arr[] = {9.32156, 3.1415926};
    json_printf(&out, "%M", json_printf_array, arr, sizeof(arr), sizeof(arr[0]),
                "%.2lf");
    ASSERT(strcmp(buf, "[9.32, 3.14]") == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    unsigned short arr[] = {65535, 777};
    const char *result = "{\"a\": [-1, 777], \"b\": 37}";
    json_printf(&out, "{a: %M, b: %d}", json_printf_array, arr, sizeof(arr),
                sizeof(arr[0]), "%hd", 37);
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *result = "{\"a\": \"\\\"\\\\\\r\\nя\\t\\u0002\"}";
    json_printf(&out, "{a: %Q}", "\"\\\r\nя\t\x02");
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    const char *arr[] = {"hi", "there", NULL};
    const char *result = "[\"hi\", \"there\", null]";
    json_printf(&out, "%M", json_printf_array, arr, sizeof(arr), sizeof(arr[0]),
                "%Q");
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    struct my_struct mys = {1, 2};
    const char *result = "{\"foo\": {\"a\": 1, \"b\": 2}, \"bar\": 3}";
    json_printf(&out, "{foo: %M, bar: %d}", print_my_struct, &mys, 3);
    ASSERT(strcmp(buf, result) == 0);
  }

  {
    struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
    out.u.buf.size = 3;
    memset(buf, 0, sizeof(buf));
    ASSERT(json_printf(&out, "{%d}", 123) == 5);
    ASSERT(memcmp(buf, "{1\x00\x00\x00", 5) == 0);
  }

  return NULL;
}

static const char *test_system() {
  char buf[2020];
  uint64_t u = 0xdeadbeeffee1dead;
  int64_t d = (int64_t) u;
  int res;

  snprintf(buf, sizeof(buf), "%" UINT64_FMT, u);
  ASSERT(strcmp(buf, "16045690985373621933") == 0);

  snprintf(buf, sizeof(buf), "%" INT64_FMT, d);
  ASSERT(strcmp(buf, "-2401053088335929683") == 0);

  res = snprintf(buf, 3, "foo");
  ASSERT(res == 3);
  ASSERT(buf[0] == 'f');
  ASSERT(buf[1] == 'o');
  ASSERT(buf[2] == '\0');

  return NULL;
}

static void cb(void *data, const char *path, const struct json_token *token) {
  char *buf = (char *) data;
  sprintf(buf + strlen(buf), "%d->%s[%.*s] ", token->type, path, token->len,
          token->ptr);
}

static const char *test_callback_api() {
  const char *s = "{\"c\":[{\"a\":9,\"b\":\"x\"}]}";
  const char *result =
      "2->.c[0].a[9] 1->.c[0].b[x] 3->.c[0][{\"a\":9,\"b\":\"x\"}] "
      "7->.c[[{\"a\":9,\"b\":\"x\"}]] 3->[{\"c\":[{\"a\":9,\"b\":\"x\"}]}] ";
  char buf[200] = "";
  ASSERT(json_parse(s, strlen(s), cb, buf) == (int) strlen(s));
  ASSERT(strcmp(buf, result) == 0);
  return NULL;
}

static void scan_array(const char *str, int len, void *user_data) {
  struct json_token t;
  int i;
  char *buf = (char *) user_data;
  for (i = 0; json_scanf_array_elem(str, len, ".x", i, &t) > 0; i++) {
    sprintf(buf + strlen(buf), "%d[%.*s] ", i, t.len, t.ptr);
  }
}

static const char *test_scanf(void) {
  char buf[100] = "";
  int a = 0, b = 0;
  char *d = NULL;
  const char *str =
      "{ a: 1234, b : true, \"c\": {x: [17, 78, -20]}, d: \"hi%20there\" }";

  ASSERT(json_scanf(str, strlen(str), "{a: %d, b: %B, c: [%M], d: %Q}", &a, &b,
                    &scan_array, buf, &d) == 4);
  ASSERT(a == 1234);
  ASSERT(b == 1);
  ASSERT(strcmp(buf, "0[17] 1[78] 2[-20] ") == 0);
  ASSERT(d != NULL);
  ASSERT(strcmp(d, "hi%20there") == 0);
  free(d);

  {
    /* Test errors */
    const char *str = "{foo:1, bar:[2,3,4]}";
    ASSERT(json_parse(str, strlen(str), NULL, NULL) == (int) strlen(str));
    for (size_t i = 1; i < strlen(str); i++) {
      ASSERT(json_parse(str, i, NULL, NULL) == JSON_STRING_INCOMPLETE);
    }
  }

  {
    /* Test that paths are utf8 */
    const char *str = "{\"ы\": 123}";
    int x = 0;
    ASSERT(json_scanf(str, strlen(str), "{ы: %d}", &x) == 1);
    ASSERT(x == 123);
  }

  {
    /* Test that paths are utf8 */
    const char *str = "{a: 123, b: [1,2,3]}";
    struct json_token t;
    memset(&t, 0, sizeof(t));
    ASSERT(json_scanf(str, strlen(str), "{b: %T}", &t) == 1);
    ASSERT(t.type == JSON_TYPE_ARRAY);
    ASSERT(t.len == 7);
    ASSERT(strncmp(t.ptr, "[1,2,3]", t.len) == 0);
  }

  {
    /* Test zero termination */
    char *s = NULL;
    const char *str = "{a: \"foo\", b:123}";
    ASSERT(json_scanf(str, strlen(str), "{a: %Q}", &s) == 1);
    ASSERT(s != NULL);
    ASSERT(s[3] == '\0');
  }

  {
    /* Test for scalar value being a root element */
    int n = 0;
    const char *str = " true ";
    ASSERT(json_scanf(str, strlen(str), " %B ", &n) == 1);
    ASSERT(n == 1);
  }

  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_scanf);
  RUN_TEST(test_errors);
  RUN_TEST(test_json_printf);
  RUN_TEST(test_system);
  RUN_TEST(test_callback_api);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
