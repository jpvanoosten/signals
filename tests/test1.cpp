#include <signals.hpp>
#include <gtest/gtest.h>

#include <iostream>

using namespace sig;
using namespace std::placeholders;

void void_func() noexcept
{
    std::cout << "Function 1" << std::endl;
    return;
}

void void_func2() noexcept
{
    std::cout << "Function 2" << std::endl;
}

int sum(int i, int j)
{
    return i + j;
}

auto lambda = [](int i, int j) { return i + j; };

struct Functor
{
    Functor(int i, int j)
        : m_i(i)
        , m_j(j)
    {}

    int sum() const noexcept
    {
        return m_i + m_j;
    }

    int m_i, m_j;
};

class Base
{
public:
    Base(int i, int j)
        : m_i(i)
        , m_j(j)
    {}

    int multiply(int i, int j) const noexcept
    {
        return i * j;
    }

    virtual int sum() const noexcept
    {
        return m_i + m_j;
    }

protected:
    int m_i, m_j;
};

class Derived : public Base
{
public:
    Derived(int i, int j)
        : Base(i, j)
    {}

    virtual int sum() const noexcept override
    {
        return Base::sum() + 1;
    }
};

TEST(slot, VoidFunc)
{
    auto s1 = make_slot(&void_func);
    auto s2 = make_slot(&void_func);
    auto s3 = make_slot(&void_func2);

    s1();
    s2();
    s3();

    EXPECT_TRUE(s1 == s2);
    EXPECT_TRUE(s2 != s3);
}

TEST(slot, Int)
{
    auto s = make_slot(&sum);
    auto res = s(3, 5);

    EXPECT_EQ(res, 8);
}

TEST(slot, MemFunc)
{
    Functor f(3, 5);
    auto s1 = make_slot(&Functor::sum, &f);

    auto res = s1();
    EXPECT_EQ(res, 8);

    auto s2 = make_slot(&Functor::sum, &f);

    EXPECT_TRUE(s1 == s2);
}

TEST(slot, Lambda)
{
    auto s1 = make_slot(lambda);
    auto s2 = make_slot(lambda);

    auto res = s1(3, 5);
    EXPECT_EQ(res, 8);

    EXPECT_TRUE(s1 == s2);
}

TEST(slot, BindLambda)
{
    auto s1 = make_slot(std::bind(lambda, 3, 5));
    auto s2 = make_slot(std::bind(lambda, _1, _2));
    auto s3 = make_slot(std::bind(lambda, 3, 5));

    {
        auto res = s1();
        EXPECT_EQ(res, 8);
    }
    {
        auto res = s2(4, 5);
        EXPECT_EQ(res, 9);
    }

    EXPECT_TRUE(s1 == s3);
    EXPECT_TRUE(s1 != s2);
}

TEST(slot, PartialBindLambda)
{
    auto s = make_slot(std::bind(lambda, _1, 5));

    auto res = s(3);
    EXPECT_EQ(res, 8);
}

TEST(slot, Virtual)
{
    Base* b = new Derived(3, 5);

    auto s = make_slot(&Base::sum, b);

    auto res = s();
    EXPECT_EQ(res, 9);

    delete b;
}

TEST(slot, RefWrap)
{
    Base* b = new Derived(3, 5);

    auto s = make_slot(&Base::sum, std::ref(*b));

    auto res = s();
    EXPECT_EQ(res, 9);

    delete b;
}

TEST(slot, CRefWrap)
{
    Base* b = new Derived(3, 5);

    auto s = make_slot(&Base::sum, std::cref(*b));

    auto res = s();
    EXPECT_EQ(res, 9);

    delete b;
}

TEST(slot, Bind)
{
    Derived d(3, 5);

    auto s = make_slot(std::bind(&Derived::multiply, &d, 3, 5));

    auto res = s();
    EXPECT_EQ(res, 15);
}

TEST(slot, PartialBind)
{
    Derived d(3, 5);

    auto s = make_slot(std::bind(&Derived::multiply, &d, _1, 5));

    auto res = s(3);
    EXPECT_EQ(res, 15);
}