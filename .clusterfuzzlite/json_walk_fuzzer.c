#include <stdint.h>
#include <frozen.h>

int 
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    json_walk((const char*)data, size, NULL, NULL);
    return 0;
}
