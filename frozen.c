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

#include <stdio.h>
#include "frozen.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

struct frozen {
  const char *end;
  const char *cur;
  struct json_token *tokens;
  int max_tokens;
  int num_tokens;
};

static int parse_object(struct frozen *f);
static int parse_value(struct frozen *f);

#define EXPECT(cond, err_code) do { if (!(cond)) return (err_code); } while (0)
#define TRY(expr) do { int n = expr; if (n < 0) return n; } while (0)
#define END_OF_STRING (-1)

static int left(const struct frozen *f) {
  return f->end - f->cur;
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

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
      return len < 6 ? JSON_STRING_INCOMPLETE :
        is_hex_digit(s[1]) && is_hex_digit(s[2]) &&
        is_hex_digit(s[3]) && is_hex_digit(s[4]) ? 5 : JSON_STRING_INVALID;
    case '"': case '\\': case '/': case 'b':
    case 'f': case 'n': case 'r': case 't':
      return len < 2 ? JSON_STRING_INCOMPLETE : 1;
    default:
      return JSON_STRING_INVALID;
  }
}

static int capture_ptr(struct frozen *f, const char *ptr, enum json_type type) {
  if (f->tokens == 0 || f->max_tokens == 0) return 0;
  if (f->num_tokens >= f->max_tokens) return JSON_TOKEN_ARRAY_TOO_SMALL;
  f->tokens[f->num_tokens].ptr = ptr;
  f->tokens[f->num_tokens].type = type;
  f->num_tokens++;
  return 0;
}

static int capture_len(struct frozen *f, int token_index, const char *ptr) {
  if (f->tokens == 0 || f->max_tokens == 0) return 0;
  EXPECT(token_index >= 0 && token_index < f->max_tokens, JSON_STRING_INVALID);
  f->tokens[token_index].len = ptr - f->tokens[token_index].ptr;
  f->tokens[token_index].num_desc = (f->num_tokens - 1) - token_index;
  return 0;
}

// identifier = letter { letter | digit | '_' }
static int parse_identifier(struct frozen *f) {
  EXPECT(is_alpha(cur(f)), JSON_STRING_INVALID);
  TRY(capture_ptr(f, f->cur, JSON_TYPE_STRING));
  while (f->cur < f->end &&
         (*f->cur == '_' || is_alpha(*f->cur) || is_digit(*f->cur))) {
    f->cur++;
  }
  capture_len(f, f->num_tokens - 1, f->cur);
  return 0;
}

static int get_utf8_char_len(unsigned char ch) {
  if ((ch & 0x80) == 0) return 1;
  switch (ch & 0xf0) {
    case 0xf0: return 4;
    case 0xe0: return 3;
    default: return 2;
  }
}

// string = '"' { quoted_printable_chars } '"'
static int parse_string(struct frozen *f) {
  int n, ch = 0, len = 0;
  TRY(test_and_skip(f, '"'));
  TRY(capture_ptr(f, f->cur, JSON_TYPE_STRING));
  for (; f->cur < f->end; f->cur += len) {
    ch = * (unsigned char *) f->cur;
    len = get_utf8_char_len((unsigned char) ch);
    //printf("[%c] [%d]\n", ch, len);
    EXPECT(ch >= 32 && len > 0, JSON_STRING_INVALID);  // No control chars
    EXPECT(len < left(f), JSON_STRING_INCOMPLETE);
    if (ch == '\\') {
      EXPECT((n = get_escape_len(f->cur + 1, left(f))) > 0, n);
      len += n;
    } else if (ch == '"') {
      capture_len(f, f->num_tokens - 1, f->cur);
      f->cur++;
      break;
    };
  }
  return ch == '"' ? 0 : JSON_STRING_INCOMPLETE;
}

// number = [ '-' ] digit+ [ '.' digit+ ] [ ['e'|'E'] ['+'|'-'] digit+ ]
static int parse_number(struct frozen *f) {
  int ch = cur(f);
  TRY(capture_ptr(f, f->cur, JSON_TYPE_NUMBER));
  if (ch == '-') f->cur++;
  EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
  EXPECT(is_digit(f->cur[0]), JSON_STRING_INVALID);
  while (f->cur < f->end && is_digit(f->cur[0])) f->cur++;
  if (f->cur < f->end && f->cur[0] == '.') {
    f->cur++;
    EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
    EXPECT(is_digit(f->cur[0]), JSON_STRING_INVALID);
    while (f->cur < f->end && is_digit(f->cur[0])) f->cur++;
  }
  if (f->cur < f->end && (f->cur[0] == 'e' || f->cur[0] == 'E')) {
    f->cur++;
    EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
    if ((f->cur[0] == '+' || f->cur[0] == '-')) f->cur++;
    EXPECT(f->cur < f->end, JSON_STRING_INCOMPLETE);
    EXPECT(is_digit(f->cur[0]), JSON_STRING_INVALID);
    while (f->cur < f->end && is_digit(f->cur[0])) f->cur++;
  }
  capture_len(f, f->num_tokens - 1, f->cur);
  return 0;
}

// array = '[' [ value { ',' value } ] ']'
static int parse_array(struct frozen *f) {
  int ind;
  TRY(test_and_skip(f, '['));
  TRY(capture_ptr(f, f->cur - 1, JSON_TYPE_ARRAY));
  ind = f->num_tokens - 1;
  while (cur(f) != ']') {
    TRY(parse_value(f));
    if (cur(f) == ',') f->cur++;
  }
  TRY(test_and_skip(f, ']'));
  capture_len(f, ind, f->cur);
  return 0;
}

static int compare(const char *s, const char *str, int len) {
  int i = 0;
  while (i < len && s[i] == str[i]) i++;
  return i == len ? 1 : 0;
}

// value = 'null' | 'true' | 'false' | number | string | array | object
static int parse_value(struct frozen *f) {
  int ch = cur(f);
  if (ch == '"') {
    TRY(parse_string(f));
  } else if (ch == '{') {
    TRY(parse_object(f));
  } else if (ch == '[') {
    TRY(parse_array(f));
  } else if (ch == 'n' && left(f) > 4 && compare(f->cur, "null", 4)) {
    TRY(capture_ptr(f, f->cur, JSON_TYPE_NULL));
    f->cur += 4;
    capture_len(f, f->num_tokens - 1, f->cur);
  } else if (ch == 't' && left(f) > 4 && compare(f->cur, "true", 4)) {
    TRY(capture_ptr(f, f->cur, JSON_TYPE_TRUE));
    f->cur += 4;
    capture_len(f, f->num_tokens - 1, f->cur);
  } else if (ch == 'f' && left(f) > 5 && compare(f->cur, "false", 5)) {
    TRY(capture_ptr(f, f->cur, JSON_TYPE_FALSE));
    f->cur += 5;
    capture_len(f, f->num_tokens - 1, f->cur);
  } else if (is_digit(ch) ||
             (ch == '-' && f->cur + 1 < f->end && is_digit(f->cur[1]))) {
    TRY(parse_number(f));
  } else {
    return ch == END_OF_STRING ? JSON_STRING_INCOMPLETE : JSON_STRING_INVALID;
  }
  return 0;
}

// key = identifier | string
static int parse_key(struct frozen *f) {
  int ch = cur(f);
#if 0
  printf("%s 1 [%.*s]\n", __func__, (int) (f->end - f->cur), f->cur);
#endif
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
  int ind;
  TRY(test_and_skip(f, '{'));
  TRY(capture_ptr(f, f->cur - 1, JSON_TYPE_OBJECT));
  ind = f->num_tokens - 1;
  while (cur(f) != '}') {
    TRY(parse_pair(f));
    if (cur(f) == ',') f->cur++;
  }
  TRY(test_and_skip(f, '}'));
  capture_len(f, ind, f->cur);
  return 0;
}

// json = object
int parse_json(const char *s, int s_len, struct json_token *arr, int arr_len) {
  struct frozen frozen = { s + s_len, s, arr, arr_len, 0 };
  if (s == 0 || s_len < 0) return JSON_STRING_INVALID;
  if (s_len == 0) return JSON_STRING_INCOMPLETE;
  TRY(parse_object(&frozen));
  TRY(capture_ptr(&frozen, frozen.cur, JSON_TYPE_EOF));
  capture_len(&frozen, frozen.num_tokens, frozen.cur);

  return frozen.cur - s;
}

static int path_part_len(const char *p) {
  int i = 0;
  while (p[i] != '\0' && p[i] != '[' && p[i] != '.') i++;
  return i;
}

const struct json_token *find_json_token(const struct json_token *toks,
                                         const char *path) {
  if (path == 0 || path[0] == '\0') return 0;
  for (;;) {
    int i, ind2 = 0, ind = -1, skip = 2, n = path_part_len(path);
    if (path[0] == '\0') return 0;
    if (path[0] == '[') {
      if (toks->type != JSON_TYPE_ARRAY || !is_digit(path[1])) return 0;
      for (ind = 0, n = 1; path[n] != ']' && path[n] != '\0'; n++) {
        if (!is_digit(path[n])) return 0;
        ind *= 10;
        ind += path[n] - '0';
      }
      if (path[n++] != ']') return 0;
      skip = 1;  // In objects, we skip 2 elems while iterating, in arrays 1.
    } else if (toks->type != JSON_TYPE_OBJECT) return 0;
    toks++;
    for (i = 0; i < toks[-1].num_desc; i += skip, ind2++) {
      // ind == -1 indicated that we're iterating an array, not object
      if (ind == -1 && toks[i].type != JSON_TYPE_STRING) return 0;
      if (ind2 == ind ||
          (ind == -1 && toks[i].len == n && compare(path, toks[i].ptr, n))) {
        i += skip - 1;
        break;
      };
      if (toks[i - 1 + skip].type == JSON_TYPE_ARRAY ||
          toks[i - 1 + skip].type == JSON_TYPE_OBJECT) {
        i += toks[i - 1 + skip].num_desc;
      }
    }
    if (i == toks[-1].num_desc) return 0;
    path += n;
    if (path[0] == '.') path++;
    if (path[0] == '\0') return &toks[i];
    toks += i;
  }
  return 0;
}

int json_emit_int(char *buf, int buf_len, long int value) {
  return buf_len <= 0 ? 0 : snprintf(buf, buf_len, "%ld", value);
}

int json_emit_double(char *buf, int buf_len, double value) {
  return buf_len <= 0 ? 0 : snprintf(buf, buf_len, "%g", value);
}

int json_emit_quoted_str(char *buf, int buf_len, const char *str) {
  int i = 0, j = 0, ch;

#define EMIT(x) do { if (j < buf_len) buf[j++] = x; } while (0)

  EMIT('"');
  while ((ch = str[i++]) != '\0' && j < buf_len) {
    switch (ch) {
      case '"':  EMIT('\\'); EMIT('"'); break;
      case '\\': EMIT('\\'); EMIT('\\'); break;
      case '\b': EMIT('\\'); EMIT('b'); break;
      case '\f': EMIT('\\'); EMIT('f'); break;
      case '\n': EMIT('\\'); EMIT('n'); break;
      case '\r': EMIT('\\'); EMIT('r'); break;
      case '\t': EMIT('\\'); EMIT('t'); break;
      default: EMIT(ch);
    }
  }
  EMIT('"');
  EMIT(0);

  return j == 0 ? 0 : j - 1;
}

int json_emit_raw_str(char *buf, int buf_len, const char *str) {
  return buf_len <= 0 ? 0 : snprintf(buf, buf_len, "%s", str);
}
