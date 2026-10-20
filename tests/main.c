#include <stdio.h>
#include "test.h"

int test_checks;
int test_failures;

void test_split(void);

int main(void)
{
    test_split();

    printf("%d checks, %d failures\n", test_checks, test_failures);
    return test_failures != 0;
}
