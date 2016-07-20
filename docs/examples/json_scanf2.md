---
title: json_scanf() complex array
---

```c
  // str has the following JSON string - array of objects:
  // { "a": [ {"b": 123}, {"b": 345} ] }
  // This example shows how to iterate over array, and parse each object.

  int i, value, len = strlen(str);
  struct json_token t;

  for (i = 0; json_scanf_array_elem(str, len, ".a", i, &t) > 0; i++) {
    // t.type == JSON_TYPE_OBJECT
    json_scanf(t.ptr, t.len, "{b: %d}", &value);  // value is 123, then 345
  }
```
