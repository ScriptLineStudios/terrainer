# terrainer
An up to date Minecraft terrain height tool

```C
//Search the first 1M seeds 7 height dirt columns at 0 0

#include <stdio.h>
#include <stdint.h>

#include "height.c"

int main(void) {
    for (uint64_t seed = 0; seed < 1000000; seed++) {
        int height = height_at(seed, 0, 0);
        if (height == 7) {
            printf("%ld\n", seed);
        }
    }

    return 0;
}
```
