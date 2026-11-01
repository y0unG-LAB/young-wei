#ifndef TEST_UTIL_H
#define TEST_UTIL_H

/* 极简测试框架，不依赖任何第三方库。
 * 用法：
 *     TC_BEGIN("用例名");
 *     CHECK(cond);
 *     CHECK_INT(actual, expected);
 *     CHECK_STR(actual, expected);
 *     TC_END();
 * 任何一条断言失败都不会中断程序（不像 assert），
 * 这样一次运行能看到所有问题，而不是改一个跑一次。
 */

#include <stdio.h>
#include <string.h>

static int t_cases  = 0;   /* 已跑用例数 */
static int t_checks = 0;   /* 已跑断言数 */
static int t_fails  = 0;   /* 失败断言总数 */
static int t_cur_fail = 0; /* 当前用例的失败数 */

#define FAIL_HEAD() printf("\n      FAIL line %d: ", __LINE__)

#define TC_BEGIN(name)                                                     \
    do {                                                                   \
        t_cases++;                                                         \
        t_cur_fail = 0;                                                    \
        printf("  %2d. %-52s", t_cases, (name));                           \
        fflush(stdout);                                                    \
    } while (0)

#define TC_END()                                                           \
    do {                                                                   \
        if (t_cur_fail == 0) {                                             \
            printf("ok\n");                                                \
        } else {                                                           \
            printf("\n      => FAILED\n");                                 \
        }                                                                  \
    } while (0)

#define CHECK(cond)                                                        \
    do {                                                                   \
        t_checks++;                                                        \
        if (!(cond)) {                                                     \
            t_fails++;                                                     \
            t_cur_fail++;                                                  \
            FAIL_HEAD();                                                   \
            printf("%s", #cond);                                           \
        }                                                                  \
    } while (0)

#define CHECK_INT(actual, expected)                                        \
    do {                                                                   \
        long _a = (long)(actual);                                          \
        long _e = (long)(expected);                                        \
        t_checks++;                                                        \
        if (_a != _e) {                                                    \
            t_fails++;                                                     \
            t_cur_fail++;                                                  \
            FAIL_HEAD();                                                   \
            printf("%s == %ld, expected %ld", #actual, _a, _e);            \
        }                                                                  \
    } while (0)

#define CHECK_STR(actual, expected)                                        \
    do {                                                                   \
        const char *_a = (actual);                                         \
        const char *_e = (expected);                                       \
        t_checks++;                                                        \
        if (_a == NULL || strcmp(_a, _e) != 0) {                           \
            t_fails++;                                                     \
            t_cur_fail++;                                                  \
            FAIL_HEAD();                                                   \
            printf("%s = \"%s\", expected \"%s\"", #actual,                \
                   (_a != NULL) ? _a : "(null)", _e);                      \
        }                                                                  \
    } while (0)

static int t_summary(void)
{
    printf("\n  ------------------------------------------\n");
    printf("  cases: %d   checks: %d   failed: %d\n", t_cases, t_checks, t_fails);
    if (t_fails == 0) {
        printf("  RESULT: ALL PASSED\n");
        return 0;
    }
    printf("  RESULT: %d CHECK(S) FAILED\n", t_fails);
    return 1;
}

#endif /* TEST_UTIL_H */
