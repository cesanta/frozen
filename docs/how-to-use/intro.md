---
title: "How to use it"
---

   * Copy `frozen.c` and `frozen.h` to your project
   * Add `frozen.c` to the list of source files
   * Parsing with Frozen is done in two steps: first, split JSON string into
      tokens by `parse_json()` or `parse_json2()`.  Second, search for certain
      parameters in parsed string by `find_json_token()`. Below is an example,
      error handling is omitted for clarity:

```c
#include <stdio.h>
#include "frozen.h"

int main(void) {
  static const char *json = " { foo: 1, bar: 2 } ";
  struct json_token *arr, *tok;

  // Tokenize json string, fill in tokens array
  arr = parse_json2(json, strlen(json));

  // Search for parameter "bar" and print it's value
  tok = find_json_token(arr, "bar");
  printf("Value of bar is: [%.*s]\n", tok->len, tok->ptr);

  // Do not forget to free allocated tokens array
  free(arr);

  return 0;
}
```
