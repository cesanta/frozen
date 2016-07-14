---
title: json_printf() example
---


Note keys are not escaped. `json_printf()` escapes them.

```c
  json_printf(&out, "{%Q: %d, x: [%B, %B], y: %Q}", "foo", 123, 0, -1, "hi");
  // Result:
  // {"foo": 123, "x": [false, true], "y": "hi"}
```

To print a complex object (for example, serialize a structure into an object),
use `%M` format specifier:

```c
  struct my_struct { int a, b; } mys = {1,2};
  json_printf(&out, "{foo: %M, bar: %d}", print_my_struct, &mys, 3);
  // Result:
  // {"foo": {"a": 1, "b": 2}, "bar": 3}
```

```c
int print_my_struct(struct json_out *out, va_list *ap) {
  struct my_struct *p = va_arg(*ap, struct my_struct *);
  return json_printf(out, "{a: %d, b: %d}", p->a, p->b);
}
```
