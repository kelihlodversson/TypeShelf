#include <stdio.h>
#include "test.h"

int test_checks;
int test_failures;

void test_split(void);
void test_group(void);
void test_decode(void);
void test_about(void);
void test_layout(void);
void test_build(void);
void test_ui(void);

int main(void)
{
    test_split();
    test_group();
    test_decode();
    test_about();
    test_layout();
    test_build();
    test_ui();

    printf("%d checks, %d failures\n", test_checks, test_failures);
    return test_failures != 0;
}
