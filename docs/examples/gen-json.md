---
title: "Generating JSON string"
---

```c
char buf[1000];
json_emit(buf, sizeof(buf), "{ s: [i, T, F, N] }", "foo", (long) -123);
```

Output: `{ "foo": [-123, true, false, null] }`
