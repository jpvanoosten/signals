#include "tests_common.hpp"
#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

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

// Add to the atomic value.
void atomic_add(std::atomic_uint64_t& i, int j)
{
    i += j;
}

using add_signal = sig::signal<void(std::atomic_uint64_t&, int)>;

// Invoke a signal 100 times.
void invoke_many(std::atomic_uint64_t& counter, add_signal& s)
{
    for (int i = 0; i < 100; ++i)
    {
        s(counter, 1);
    }
}

// Test multi-threaded signal emission.
TEST(signal, InvokeThreaded)
{
    add_signal s;
    s.connect(atomic_add);

    std::atomic_uint64_t i{0};

    std::array<std::thread, 10> threads;
    for (auto& t : threads)
    {
        t = std::thread(invoke_many, std::ref(i), std::ref(s));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(i, 1000);
}

void connect_invoke(std::atomic_uint64_t& counter, add_signal& s)
{
    for (int i = 0; i < 100; ++i)
    {
        auto sc = s.connect_scoped(atomic_add);
        for (int j = 0; j < 100; ++j)
        {
            s(counter, 1);
        }
    }
}

// Test threaded connection and emission on the same signal.
TEST(signal, ThreadedConnect)
{
    add_signal s;

    std::atomic_uint64_t i{0};

    connect_invoke(i, s);

    // No more signals should be connected.
    s(i, 1);

    EXPECT_EQ(i, 10000);

    i = 0;

    std::array<std::thread, 10> threads;
    for (auto& t : threads)
    {
        t = std::thread(connect_invoke, std::ref(i), std::ref(s));
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

void connect_cross(add_signal& s1, add_signal& s2, std::atomic_uint64_t& counter, std::atomic_int& go)
{
    auto cross = s1.connect([&](std::atomic_uint64_t& c, int i)
    {
        if (i & 1)
        {
            atomic_add(c, i);
        }
        else
        {
            s2(c, i + 1);
        }
    });

    go++;
    while (go != 3)
        std::this_thread::yield();

    for (int i = 0; i < 1000000; ++i)
    {
        s1(counter, i);
    }
}

// Test for deadlocks in cross-emission situation.
TEST(signal, ThreadedCrossed)
{
    std::atomic_uint64_t i{0};

    add_signal s1;
    add_signal s2;

    std::atomic_int go{0};

    std::thread t1(connect_cross, std::ref(s1), std::ref(s2), std::ref(i), std::ref(go));
    std::thread t2(connect_cross, std::ref(s2), std::ref(s1), std::ref(i), std::ref(go));

    while ( go != 2 )
        std::this_thread::yield();

    go++;

    t1.join();
    t2.join();

    EXPECT_EQ(i, 1000000000000ll);
}

// A combiner that returns the maximum value returned by the slots,
// or a default constructed value T.
template<typename T>
struct max_or_default
{
    using result_type = T;

    template<typename InputIterator>
    typename InputIterator::value_type operator()(InputIterator first, InputIterator last) const
    {
        opt::optional<T> max;
        while (first != last)
        {
            opt::optional<T> tmp = *first;
            if (tmp > max) max = tmp;

            ++first;
        }

        return max ? *max : T();
    }
};

struct make_int
{
    make_int(int n, int cn )
    : N(n)
    , CN(cn)
    {}

    int operator()()
    {
        return N;
    }

    int operator()() const
    {
        return CN;
    }

    bool operator==(const make_int& other) const
    {
        return N == other.N && CN == other.CN;
    }

    int N;
    int CN;
};

template<int N>
struct make_increasing_int
{
    make_increasing_int()
        : n(N)
    {}

    int operator()() const
    {
        return n++;
    }

    mutable int n;
};

TEST(signal, ZeroArgs)
{
    using signal = sig::signal<int(), max_or_default<int>>;
    
    make_int i42(42, 41);
    make_int i2(2, 1);
    make_int i72(72, 71);
    make_int i63(63, 63);
    make_int i62(62, 61);

    {
        signal s;

        auto c2 = s.connect(i2);
        auto c72 = s.connect(i72);
        auto c62 = s.connect(i62);
        auto c42 = s.connect(i42);

        EXPECT_EQ(s(), 72);

        s.disconnect(i72);
        EXPECT_EQ(s(), 62);

        c72.disconnect();   // This should do nothing (it's already disconnected)
        EXPECT_EQ(s(), 62);

        auto c63 = s.connect(i63);
        EXPECT_EQ(s(), 63);

        s.disconnect(i63);
        s.disconnect(i62);
        EXPECT_EQ(s(), 42);

        c42.disconnect();
        EXPECT_EQ(s(), 2);

        c2.disconnect();
        EXPECT_EQ(s(), 0);
    }

    {
        signal s;
        
        auto c2 = s.connect(i2);
        auto c72 = s.connect(i72);
        auto c62 = s.connect(i62);
        auto c42 = s.connect(i42);

        const signal& cs = s;
        EXPECT_EQ(cs(), 72);
    }

    {
        make_increasing_int<7> i7;
        make_increasing_int<10> i10;

        signal s;

        auto c7 = s.connect(i7);
        auto c10 = s.connect(i10);

        EXPECT_EQ(s(), 10);
        EXPECT_EQ(s(), 11);
    }
}

TEST(signal, OneArg)
{
    using signal = sig::signal<int(int value), max_or_default<int>>;

    signal s;

    s.connect(std::negate<int>());
    s.connect(std::bind(std::multiplies<int>(), 2, _1));

    EXPECT_EQ(s(1), 2);
    EXPECT_EQ(s(-1), 1);
}

TEST(signal, ConnectSlot)
{
    using signal = sig::signal<void()>;
    using slot = signal::slot_type;

    // Create a signal.
    signal s;

    // Create a slot for void_func.
    slot sl(&void_func);

    // Connect the slot to the signal.
    s.connect(sl);

    // Invoke the signal.
    s();

    // Disconnect the slot.
    s.disconnect(sl);

    // Invoke the signal again.
    s();
}