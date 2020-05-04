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
    s.connect(&void_func2); // Connect void_func2 to test disconnect.

    auto res = s();

    // The result of void function is an opt::optional<void> which is always
    // a disengaged optional.
    EXPECT_FALSE(res);

    // void_func was connected once, disconnecting it should result in a count
    // of 1.
    auto count = s.disconnect(&void_func);

    EXPECT_EQ(count, 1);

    count = s.disconnect(&void_func2);

    // void_func2 was connected twice. 
    // Disconnecting it will result in 2 slots being removed.
    EXPECT_EQ(count, 2);

    // All slots have been disconnected.
    // Invoking the signal should still work.
    res = s();

    // Calling a signal with 0 slots should also result
    // in a disengaged optional value.
    EXPECT_FALSE(res);
}

TEST(signal, VoidConnection)
{
    using signal = sig::signal<void()>;

    signal s;

    // Store the connection.
    auto c1 = s.connect(&void_func);
    auto c2 = s.connect(&void_func2);

    auto res = s();

    EXPECT_FALSE(res);

    // Disconnect signal from slot.
    // connection::disconnect should return true.
    EXPECT_TRUE(c1.disconnect());
    EXPECT_TRUE(c2.disconnect());

    // Disconnecting again should not do anything.
    // but the connection::disconnect method should return false.
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
        // Scoped connections will automatically disconnect the slot.
        auto c1 = s.connect_scoped(&void_func);
        auto c2 = s.connect_scoped(&void_func2);

        auto res = s();

        EXPECT_FALSE(res);
    }

    // The slots should be disconnected now.
    auto res = s();

    EXPECT_FALSE(res);
}

TEST(signal, TestCounter)
{
    using signal = sig::signal<void(int&)>;

    signal s;

    s.connect(&increment_counter);
    s.connect(&increment_counter);
    s.connect(&increment_counter);

    int counter = 0;

    s(counter);

    EXPECT_EQ(counter, 3);

    auto count = s.disconnect(&increment_counter);

    EXPECT_EQ(count, 3);

    // No slots should be called.
    s(counter);

    // Counter remains unchanged.
    EXPECT_EQ(counter, 3);
}

