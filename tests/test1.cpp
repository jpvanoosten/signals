#include <signals2.hpp>
#include <gtest/gtest.h>

#include <iostream>

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

    int operator()() const noexcept
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
    using s = sig::slot<void(void)>;

    auto s1 = s(&void_func);
    auto s2 = s(&void_func);
    auto s3 = s(&void_func2);

    s1();
    s2();
    s3();
}

TEST(slot, Int)
{
    using s = sig::slot<int(int, int)>;

    auto s1 = s(&sum);
    auto res = s1(3, 5);

    EXPECT_EQ(res, 8);
}

//TEST(slot, MemFunc)
//{
//    Functor f(3, 5);
//    auto s1 = make_slot(&Functor::sum, &f);
//
//    auto res = s1();
//    EXPECT_EQ(res, 8);
//
//    auto s2 = make_slot(&Functor::sum, &f);
//
//    EXPECT_TRUE(s1 == s2);
//}
//
TEST(slot, Lambda)
{
    using s = sig::slot<int(int,int)>;

    auto s1 = s(lambda);
    auto s2 = s(lambda);

    auto res = s1(3, 5);
    EXPECT_EQ(res, 8);
}

TEST(slot, BindLambda)
{
    auto s1 = sig::slot<int()>(std::bind(lambda, 3, 5));
    auto s2 = sig::slot<int(int,int)>(std::bind(lambda, _1, _2));
    auto s3 = sig::slot<int()>(std::bind(lambda, 3, 5));

    {
        auto res = s1();
        EXPECT_EQ(res, 8);
    }
    {
        auto res = s2(4, 5);
        EXPECT_EQ(res, 9);
    }
}

TEST(slot, PartialBindLambda)
{
    auto s = sig::slot<int(int)>(std::bind(lambda, _1, 5));

    auto res = s(3);
    EXPECT_EQ(res, 8);
}

//
//TEST(slot, Virtual)
//{
//    Base* b = new Derived(3, 5);
//
//    auto s = make_slot(&Base::sum, b);
//
//    auto res = s();
//    EXPECT_EQ(res, 9);
//
//    delete b;
//}
//
//TEST(slot, RefWrap)
//{
//    Base* b = new Derived(3, 5);
//
//    auto s = make_slot(&Base::sum, std::ref(*b));
//
//    auto res = s();
//    EXPECT_EQ(res, 9);
//
//    delete b;
//}
//
//TEST(slot, CRefWrap)
//{
//    Base* b = new Derived(3, 5);
//
//    auto s = make_slot(&Base::sum, std::cref(*b));
//
//    auto res = s();
//    EXPECT_EQ(res, 9);
//
//    delete b;
//}
//
TEST(slot, Bind)
{
    Derived d(3, 5);

    auto s = sig::slot<int()>(std::bind(&Derived::multiply, &d, 3, 5));

    auto res = s();
    EXPECT_EQ(res, 15);
}

TEST(slot, PartialBind)
{
    Derived d(3, 5);

    auto s = sig::slot<int(int)>(std::bind(&Derived::multiply, &d, _1, 5));

    auto res = s(3);
    EXPECT_EQ(res, 15);
}