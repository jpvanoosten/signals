#include "tests_common.hpp"
#include <gtest/gtest.h>

#include <iostream>

using namespace std::placeholders;

TEST(slot, VoidFunc)
{
    using s = sig::slot<void(void)>;

    auto s1 = s(&void_func);
    auto s2 = s(&void_func);
    auto s3 = s(&void_func2);

    s1();
    s2();
    s3();

    // Slots pointing to the same function should be equal.
    EXPECT_TRUE(s1 == s2);

    // Slots pointing to different functions should not be equal.
    EXPECT_TRUE(s1 != s3);
    EXPECT_TRUE(s2 != s3);
}

TEST(slot, VoidMemberFunc)
{
    using s = sig::slot<void()>;

    VoidMemberFunc vmf;

    auto s1 = s(&VoidMemberFunc::DoSomething, &vmf);

    // Slot returns opt::optional<void>
    auto res = s1();

    EXPECT_FALSE(res);
}

TEST(slot, VoidMemberFuncTracked)
{
    using s = sig::slot<void()>;

    auto vmf = std::make_shared<VoidMemberFunc>();

    auto s1 = s(&VoidMemberFunc::DoSomething, vmf);

    // Slot returns opt::optional<void>
    auto res = s1();

    EXPECT_FALSE(res);

    // Release shared pointer.
    vmf.reset();

    res = s1();

    // Still disengaged optional<void>
    EXPECT_FALSE(res);
}

TEST(slot, IntFunc)
{
    using s = sig::slot<int(int, int)>;

    auto s1 = s(&sum);
    auto s2 = s(&sum);

    auto res = s1(3, 5);

    EXPECT_EQ(res, 8);

    // Slots pointing to the same function should be equal.
    EXPECT_TRUE(s1 == s2);
}

TEST(slot, Functor)
{
    auto s1 = sig::slot<int()>(Functor(3,5));
    auto s2 = sig::slot<int()>(Functor(3,5));

    auto res = s1();
    EXPECT_EQ(res, 8);

    // The Functor class does not define an equality operator.
    // Attempting to compare two slots with Functor should throw
    // an exception.
    EXPECT_THROW(s1 == s2, sig::not_comparable_exception);
}

TEST(slot, Lambda)
{
    using s = sig::slot<int(int, int)>;

    auto s1 = s(lambda);
    auto s2 = s(lambda);

    auto res = s1(3, 5);
    EXPECT_EQ(res, 8);

    // Two slots pointing to the same lambda should be true?
    EXPECT_TRUE(s1 == s2);
}

TEST(slot, VoidLambda)
{
    using s = sig::slot<void(void)>;

    auto s1 = s(void_lambda);
    auto s2 = s(void_lambda);

    auto res = s1();
    EXPECT_FALSE(res);

    // Two slots pointing to the same lambda should be true?
    EXPECT_TRUE(s1 == s2);
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

    p.reset();

    res = s();
    EXPECT_FALSE(res);
}

TEST(slot, NullSlot)
{
    // Construct an empty slot.
    auto s1 = sig::slot<void()>();
    auto s2 = sig::slot<void()>();

    // Should evaluate to false.
    EXPECT_FALSE(s1);
    EXPECT_FALSE(s2);

    // Null slots should be equality comparable.
    EXPECT_TRUE(s1 == s2);

    // Provide a valid slot.
    s1 = sig::slot<void()>(&void_func);
    s2 = sig::slot<void()>(&void_func);

    // Should evaluate to true now.
    EXPECT_TRUE(s1);
    EXPECT_TRUE(s2);

    // Slots pointing to the same function should be equal.
    EXPECT_TRUE(s1 == s2);
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