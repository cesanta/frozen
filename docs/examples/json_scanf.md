---
title: json_scanf() example
---

```c
  // str has the following JSON string (notice keys are out of order):
  // { "a": 123, "d": true, "b": [1, 2], "c": "hi" }

  int a, b;
  char *c;
  void *my_data = NULL;
  json_scanf(str, strlen(str), "{ a:%d, b:%M, c:%Q, d:%B }",
             &a, scan_array, my_data, &c, &b);


  // This function is called by json_scanf() call above.
  // str is "[1, 2]", user_data is my_data.
  static void scan_array(const char *str, int len, void *user_data) {
    struct json_token t;
    int i;
    printf("Parsing array: %.*s\n", len, str);
    for (i = 0; json_scanf_array_elem(str, len, ".x", i, &t) > 0; i++) {
      printf("Index %d, token [%.*s]\n", i, t.len, t.ptr);
    }
  }
```
