#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

extern int test_checks;
extern int test_failures;

#define CHECK(cond) do { \
    test_checks++; \
    if (!(cond)) { \
        test_failures++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_INT(got, want) do { \
    long g_ = (long)(got), w_ = (long)(want); \
    test_checks++; \
    if (g_ != w_) { \
        test_failures++; \
        printf("FAIL %s:%d: %s: got %ld, want %ld\n", \
               __FILE__, __LINE__, #got, g_, w_); \
    } \
} while (0)

#define CHECK_STR(got, want) do { \
    const char *g_ = (got), *w_ = (want); \
    test_checks++; \
    if (strcmp(g_, w_) != 0) { \
        test_failures++; \
        printf("FAIL %s:%d: %s: got \"%s\", want \"%s\"\n", \
               __FILE__, __LINE__, #got, g_, w_); \
    } \
} while (0)

#endif
