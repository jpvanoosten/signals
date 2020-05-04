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
    // All 3 slots should be removed.
    EXPECT_EQ(count, 3);

    // No slots should be called.
    s(counter);

    // Counter remains unchanged.
    EXPECT_EQ(counter, 3);
}

TEST(signal, TestConnectionBlocking)
{
    using signal = sig::signal<void(int&)>;

    signal s;

    auto c1 = s.connect(&increment_counter);
    auto c2 = s.connect(&increment_counter);
    auto c3 = s.connect(&increment_counter);

    int counter = 0;

    s(counter);

    EXPECT_EQ(counter, 3);

    // Block 2 connections.
    c1.block();
    c2.block();

    // Only 1 of the slots should be called.
    s(counter);

    // Counter only increments once.
    EXPECT_EQ(counter, 4);

    c1.unblock();
    c2.unblock();

    s(counter);

    // Should be incremented 3 times again.
    EXPECT_EQ(counter, 7);
}

TEST(signal, DefaultArguments)
{
    using signal = sig::signal<void(int&)>;

    signal s;

    // Must use a function adapter that calls the function utilizing default arguments.
    // Can also use std::function to create such a binding.
    auto func_adapter = [](int& i) { default_arguments(i); };

    // Connect the function adapter to the signal.
    s.connect(func_adapter);

    int counter = 0;

    s(counter);

    EXPECT_EQ(counter, 1);

    // Remove the slot.
    s.disconnect(func_adapter);

    s(counter);

    // Counter should remain unchanged.
    EXPECT_EQ(counter, 1);
}

TEST(signal, PointerToMemberFunction)
{
    using signal = sig::signal<void()>;
    signal s;

    VoidMemberFunc vmf;

    s.connect(&VoidMemberFunc::DoSomething, &vmf);

    // Function returns opt::optional<void> which is always disengaged.
    auto res = s();

    EXPECT_FALSE(res);
}

TEST(signal, PointerToMemberFunctionTracked)
{
    using signal = sig::signal<void()>;
    signal s;

    auto vmf = std::make_shared<VoidMemberFunc>();

    s.connect(&VoidMemberFunc::DoSomething, vmf);

    s();

    // Release smart pointer object.
    vmf.reset();

    s();

    // The slot should not prevent the shared pointer from being
    // released.
    EXPECT_EQ(vmf.use_count(), 0);

}

std::atomic_uint32_t i1;

// Add to the atomic value.
void add_i(int i)
{
    i1 += i;
}

// Invoke a signal 100 times.
void invoke_many(sig::signal<void(int)>& s)
{
    for( int i = 0; i < 100; ++i )
    {
        s(1);
    }
}

void test_int(int i)
{
    i1 += i;
}

TEST(signal, InvokeThreaded)
{
    using signal = sig::signal<void(int)>;

    signal s;
    s.connect(add_i);

    invoke_many(s);

    EXPECT_EQ(i1, 100);
}