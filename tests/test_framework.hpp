// tests/test_framework.hpp —— 极简断言宏
//
// 刻意保持朴素。它的价值在于"零依赖 + 立刻能跑"，
// 而不是功能齐全。等哪天你真的觉得不够用了，再换 gtest 也不迟。

#pragma once

#include <cstdio>

namespace cubetest {

extern int g_checks;
extern int g_failures;
extern const char* g_current_case;

void beginCase(const char* name);
void endCase();
int summary();

}  // namespace cubetest

// 建议在 applyMove 之后调用，第一时间抓到状态被污染
#define EXPECT_TRUE(expr)                                                          \
    do {                                                                           \
        ++::cubetest::g_checks;                                                    \
        if (!(expr)) {                                                             \
            ++::cubetest::g_failures;                                              \
            std::printf("  [FAIL] %s:%d  EXPECT_TRUE(%s)\n", __FILE__, __LINE__,   \
                        #expr);                                                    \
        }                                                                          \
    } while (false)

#define EXPECT_EQ(a, b)                                                            \
    do {                                                                           \
        ++::cubetest::g_checks;                                                    \
        if (!((a) == (b))) {                                                       \
            ++::cubetest::g_failures;                                              \
            std::printf("  [FAIL] %s:%d  EXPECT_EQ(%s, %s)\n", __FILE__, __LINE__, \
                        #a, #b);                                                   \
        }                                                                          \
    } while (false)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define RUN_CASE(name)         \
    do {                       \
        ::cubetest::beginCase(name); \
        name();                \
        ::cubetest::endCase(); \
    } while (false)
