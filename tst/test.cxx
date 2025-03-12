#include "gtest/gtest.h"
#include "../src/factorial.h"

TEST(FactorialValueTest, FactorialOf3)
{
    ASSERT_EQ(GetFactorial(3), 6);
}

TEST(FactorialValueTest, FactorialOf0)
{
    ASSERT_EQ(GetFactorial(0), 1);
}

TEST(FactorialValueTest, FactorialOf14)
{
    ASSERT_EQ(GetFactorial(14), 87178291200);
}