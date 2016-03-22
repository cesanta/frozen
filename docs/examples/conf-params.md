---
title: "Accessing configuration parameters"
---

```c
#include "frozen.h"

static const char *config_str = " { ports: [ 80, 443 ] } ";
struct json_token tokens[10];
int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

// Parse config string and make sure tokenization is correct
ASSERT(parse_json(config_str, strlen(config_str), tokens, tokens_size) > 0);

ASSERT(tokens[0].type == JSON_TYPE_OBJECT);   // Tokens are populated
ASSERT(tokens[1].type == JSON_TYPE_STRING);   // in order of their
ASSERT(tokens[2].type == JSON_TYPE_ARRAY);    // appearance in the
ASSERT(tokens[3].type == JSON_TYPE_NUMBER);   // JSON string
ASSERT(tokens[4].type == JSON_TYPE_NUMBER);
ASSERT(tokens[5].type == JSON_TYPE_EOF);      // Last token is always EOF

// Fetch port values
ASSERT(find_json_token(tokens, "ports") == &tokens[2]);
ASSERT(find_json_token(tokens, "ports[0]") == &tokens[3]);
ASSERT(find_json_token(tokens, "ports[1]") == &tokens[4]);
ASSERT(find_json_token(tokens, "ports[3]") == NULL);  // Outside boundaries
ASSERT(find_json_token(tokens, "foo.bar") == NULL);   // Nonexistent
```
