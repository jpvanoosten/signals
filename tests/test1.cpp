#include <signals.hpp>
#include <gtest/gtest.h>

#include <iostream>

using namespace signals;

void func()
{
    std::cout << "Func" << std::endl;
    return;
}

int sum(int i, int j)
{
    return i + j;
}

auto lambda = [](int i, int j){ return i + j; };

struct Functor
{
    Functor(int i, int j)
    : m_i(i)
    , m_j(j)
    {}

    int sum()
    {
        return m_i + m_j;
    }

    int m_i, m_j;
};

TEST(slot, TestVoid)
{
    auto s = make_slot(&func);
    s();
}

TEST(slot, TestInt)
{
    auto s = make_slot(&sum);
    auto res = s(3, 5);

    EXPECT_EQ(res, 8);
}

TEST(slot, TestMemFunc)
{
    Functor f(3, 5);
    auto s = make_slot(&Functor::sum, &f);

    auto res = s();
    EXPECT_EQ(res, 8);
}

TEST(slot, TestLambda)
{
    auto s = make_slot(lambda);

    auto res = s(3, 5);
    EXPECT_EQ(res, 8);
}
