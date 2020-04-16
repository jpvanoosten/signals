#include <signals.hpp>
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

struct PointerToMemberData
{
    PointerToMemberData(int v)
    : value(v)
    {}

    int value;
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

TEST(slot, IntFunc)
{
    using s = sig::slot<int(int, int)>;

    auto s1 = s(&sum);
    auto res = s1(3, 5);

    EXPECT_EQ(res, 8);
}

TEST(slot, Functor)
{
    auto s1 = sig::slot<int()>(Functor(3,5));

    auto res = s1();
    EXPECT_EQ(res, 8);
}

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


TEST(slot, VirtualFunc)
{
    Base* b = new Derived(3, 5);

    auto s1 = sig::slot<int()>(&Base::sum, b);
    auto s2 = sig::slot<int()>(&Base::sum, Derived(3,5));

    EXPECT_EQ(s1(), 9);
    EXPECT_EQ(s2(), 9);
    delete b;
}

TEST(slot, PointerToMemberData)
{
    PointerToMemberData pmd(5);

    auto s1 = sig::slot<int()>(&PointerToMemberData::value, &pmd);

    EXPECT_EQ(s1(), 5);
}

TEST(slot, RefWrap)
{
    Base* b = new Derived(3, 5);

    auto s = sig::slot<int()>(&Base::sum, std::ref(*b));

    auto res = s();
    EXPECT_EQ(res, 9);

    delete b;
}

TEST(slot, CRefWrap)
{
    Base* b = new Derived(3, 5);

    auto s = sig::slot<int()>(&Base::sum, std::cref(*b));

    auto res = s();
    EXPECT_EQ(res, 9);

    delete b;
}

TEST(slot, SharedPtr)
{
    auto p = std::make_shared<Derived>(3, 5);

    auto s = sig::slot<int()>(&Base::sum, p);

    auto res = s();
    EXPECT_EQ(res, 9);
}

TEST(slot, NullSlot)
{
    // Construct an empty slot.
    auto s = sig::slot<void()>();

    // Should evaluate to false.
    EXPECT_FALSE(s);

    // Provide a valid slot.
    s = sig::slot<void()>(&void_func);

    // Should evaluate to false.
    EXPECT_TRUE(s);
}

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