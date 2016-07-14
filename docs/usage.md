---
title: Usage
---

* Copy `frozen.c` and `frozen.h` to your project
* Add `frozen.c` to the list of source files

```c
#include <stdio.h>
#include <string.h>
#include "frozen.h"

int main(void) {
  const char *str = "{ \"a\": 123, \"b\": true }";
  int b = 0;

  // Fetch boolean key "b" from the JSON string
  json_scanf(str, strlen(str), "{b: %B}", &b);

  printf("Result: %d\n", b);

  return 0;
}
```

Compile and run the code:

```
$ cc main.c frozen.c
$ ./a.out
Result: 1
```
