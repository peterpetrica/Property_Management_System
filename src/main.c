#include <stdio.h>
#include <stdlib.h>

#include "factorial.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    unsigned long long factorial = GetFactorial(n);
    printf("The factorial of %d = %llu\n", n, factorial);
    return 0;
}