#include <stdio.h>
#include "test.h"

int test_checks;
int test_failures;

void test_split(void);
void test_group(void);

int main(void)
{
    test_split();
    test_group();

    printf("%d checks, %d failures\n", test_checks, test_failures);
    return test_failures != 0;
}
