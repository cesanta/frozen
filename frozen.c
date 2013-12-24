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

#include "frozen.h"


#include <stdio.h>  // TODO:remove this

struct frozen {
  const char *end;
  const char *cur;
  struct json_token *tokens;
  int max_tokens;
  int num_tokens;
};

static int parse_object(struct frozen *f);

#define EXPECT(cond, err_code) do { if (!(cond)) return (err_code); } while (0)
#define TRY(expr) do { int n = expr; if (n < 0) return n; } while (0)
#define SKIP_SPACES(f) do { skip_whitespaces(f); \
  if (f->cur >= f->end) return JSON_STRING_INCOMPLETE; } while (0)
#define END_OF_STRING (-1)

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
};

static void skip_whitespaces(struct frozen *f) {
  while (f->cur < f->end && is_space(*f->cur)) f->cur++;
}

static int cur(struct frozen *f) {
  skip_whitespaces(f);
  return f->cur >= f->end ? END_OF_STRING : * (unsigned char *) f->cur;
}

static int test_and_skip(struct frozen *f, int expected) {
  int ch = cur(f);
  if (ch == expected) { f->cur++; return 0; }
  return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
}

static int is_alpha(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int is_digit(int ch) {
  return ch >= '0' && ch <= '9';
}

static int is_hex_digit(int ch) {
  return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static int get_escape_len(const char *s, int len) {
  switch (*s) {
    case 'u':
      return len < 4 ? JSON_STRING_INCOMPLETE :
        is_hex_digit(s[0]) && is_hex_digit(s[1]) &&
        is_hex_digit(s[2]) && is_hex_digit(s[3]) ? 4 : JSON_STRING_INVALID;
    case '"': case '\\': case '/': case 'b':
    case 'f': case 'n': case 'r': case 't':
      return len < 2 ? JSON_STRING_INCOMPLETE : 2;
    default:
      return JSON_STRING_INVALID;
  }
}

// identifier = letter { letter | digit | '_' }
static int parse_identifier(struct frozen *f) {
  //printf("%s 1 [%.*s]\n", __func__, (int) (f->end - f->cur), f->cur);
  EXPECT(is_alpha(cur(f)), JSON_STRING_INVALID);
  while (f->cur < f->end &&
         (*f->cur == '_' || is_alpha(*f->cur) || is_digit(*f->cur))) {
    f->cur++;
  }
  return 0;
}

// string = '"' { quoted_printable_chars } '"'
static int parse_string(struct frozen *f) {
  int n, ch;
  TRY(test_and_skip(f, '"'));
  while (++f->cur < f->end) {
    ch = cur(f);
    EXPECT(ch > 32 && ch < 127, JSON_STRING_INVALID);
    if (ch == '\\') {
      EXPECT((n = get_escape_len(f->cur + 1, f->end - f->cur)) > 0, n);
      f->cur += n;
    } else if (ch == '"') {
      break;
    };
  }
  return 0;
}

// value = 'null' | 'true' | 'false' | number | string | array | object
static int parse_value(struct frozen *f) {
  int ch = cur(f);
  if (ch == '"') {
    TRY(parse_string(f));
  } else if (ch == '{') {
    TRY(parse_object(f));
  } else {
    return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
  }
  return 0;
}

// key = identifier | string
static int parse_key(struct frozen *f) {
  int ch = cur(f);
  //printf("%s 1 [%.*s]\n", __func__, (int) (f->end - f->cur), f->cur);
  if (is_alpha(ch)) {
    TRY(parse_identifier(f));
  } else if (ch == '"') {
    TRY(parse_string(f));
  } else {
    return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
  }
  return 0;
}

// pair = key ':' value
static int parse_pair(struct frozen *f) {
  TRY(parse_key(f));
  TRY(test_and_skip(f, ':'));
  TRY(parse_value(f));
  return 0;
}


// object = '{' pair { ',' pair } '}'
static int parse_object(struct frozen *f) {
  TRY(test_and_skip(f, '{'));
  while (cur(f) != '}') {
    TRY(parse_pair(f));
  }
  TRY(test_and_skip(f, '}'));
  return 0;
}

// number = [ '-' ] digit { digit }
// array = '[' [ value { ',' value } ] ']'
// json = object
int parse_json(const char *s, int s_len, struct json_token *arr, int arr_len) {
  struct frozen frozen = { s + s_len, s, arr, arr_len, 0 };
  if (s == 0 || s_len < 0) return JSON_STRING_INVALID;
  if (s_len == 0) return JSON_STRING_INCOMPLETE;
  TRY(parse_object(&frozen));
  return frozen.cur - s;
}
