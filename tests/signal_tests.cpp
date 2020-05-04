#include "tests_common.hpp"
#include <gtest/gtest.h>

#include <iostream>

using namespace std::placeholders;

TEST(signal, VoidFunc)
{
    using signal = sig::signal<void()>;

    signal s;
    s.connect(&void_func);
    s.connect(&void_func2);
    s.connect(&void_func2);

    auto res = s();

    // The result of void function is an opt::optional<void> which is always
    // a disengaged optional.
    EXPECT_FALSE(res);

    auto count = s.disconnect(&void_func);

    EXPECT_EQ(count, 1);

    count = s.disconnect(&void_func2);

    // void_func2 was connected twice. 
    // Disconnecting it will result in 2 slots being removed.
    EXPECT_EQ(count, 2);

    res = s();

    // Calling a signal with 0 slots should also result
    // in a disengaged optional value.
    EXPECT_FALSE(res);
}

TEST(signal, VoidConnection)
{
    using signal = sig::signal<void()>;

    signal s;
    auto c1 = s.connect(&void_func);
    auto c2 = s.connect(&void_func2);

    auto res = s();

    EXPECT_FALSE(res);

    // Disconnect signal from slot.
    EXPECT_TRUE(c1.disconnect());
    EXPECT_TRUE(c2.disconnect());

    // Disconnecting again should not do anything.
    EXPECT_FALSE(c1.disconnect());
    EXPECT_FALSE(c2.disconnect());

    res = s();

    EXPECT_FALSE(res);
}

TEST(signal, VoidScopedConnection)
{
    using signal = sig::signal<void()>;

    signal s;
    {
        auto c1 = s.connect_scoped(&void_func);
        auto c2 = s.connect_scoped(&void_func2);

        auto res = s();

        EXPECT_FALSE(res);
    }

    // The slots should be disconnected now.
    auto res = s();

    EXPECT_FALSE(res);
}