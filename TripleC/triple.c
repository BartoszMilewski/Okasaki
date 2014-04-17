#include <stdio.h>

void triples(int n)
{
    int i = 0;
    for (int z = 1; ; ++z)
        for (int x = 1; x <= z; ++x)
            for (int y = x; y <= z; ++y)
                if (x*x + y*y == z*z) {
                    printf("%d, %d, %d\n", x, y, z);
                    if (++i == n)
                        return;
                }
}

void main()
{
    triples(10);
}

