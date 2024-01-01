# terrainer
An up to date Minecraft dirt terrain height tool. Terrainer uses a modified version of cubiomes.

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

Running this gives us the following output:
```bash
...
940276
944200
948707
979242
986421
992942
999588
```

Loading up the seed 999588 in Minecraft 1.20.4 and heading to 0 0, we sure enough do find a 7 height dirt column!
![2024-01-01_13 08 56](https://github.com/ScriptLineStudios/terrainer/assets/85095943/f8aa50f1-b743-4fc5-9dc2-18b09fb49fcd)
