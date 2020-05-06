#include <signals.hpp>
#include <gtest/gtest.h>

#include "tests_common.hpp"

TEST(connection, Swap)
{
    using signal = sig::signal<void()>;

    signal s;

    auto c1 = s.connect(&void_func);
    signal::connection_type c2;
    EXPECT_TRUE(c1.connected());
    EXPECT_FALSE(c2.connected());

    c1.swap(c2);
    EXPECT_FALSE(c1.connected());
    EXPECT_TRUE(c2.connected());

    sig::swap(c1, c2);
    EXPECT_TRUE(c1.connected());
    EXPECT_FALSE(c2.connected());
}

TEST(connection, Release)
{
    using signal = sig::signal<void()>;

    signal s;

    signal::connection_type c1;
    {
        auto scoped = s.connect_scoped(&void_func);
        EXPECT_TRUE(scoped.connected());
        c1 = scoped.release();
    }
    EXPECT_TRUE(c1.connected());

    signal::connection_type c2;
    {
        signal::scoped_connection_type scoped(c1);
        EXPECT_TRUE(scoped.connected());
        c1 = scoped.release();
        EXPECT_TRUE(c1.connected());
        EXPECT_FALSE(scoped.connected());
        c1.disconnect();

        c2 = s.connect(&void_func);
        scoped = c2;
    }
    EXPECT_FALSE(c2.connected());
}

TEST(connection, Move)
{
    using signal = sig::signal<void()>;

    signal s;

    // Test move assignment from scoped_connection to connection
    signal::connection_type c1;
    {
        signal::scoped_connection_type scoped(s.connect(&void_func));
        EXPECT_TRUE(scoped.connected());
        
        c1 = std::move(scoped);
        EXPECT_FALSE(scoped.connected());
    }
    EXPECT_TRUE(c1.connected());

    // Test move construction from scoped to scoped.
    {
        signal::scoped_connection_type scoped1(c1);
        EXPECT_TRUE(scoped1.connected());
        
        signal::scoped_connection_type scoped2(std::move(scoped1));
        EXPECT_FALSE(scoped1.connected());
        EXPECT_TRUE(scoped2.connected());
        EXPECT_TRUE(c1.connected());
    }
    EXPECT_FALSE(c1.connected());

    // Test move assignment from scoped to scoped.
    c1 = s.connect(&void_func);
    {
        signal::scoped_connection_type scoped1;
        signal::scoped_connection_type scoped2(c1);
        EXPECT_TRUE(scoped2.connected());

        scoped1 = std::move(scoped2);
        EXPECT_FALSE(scoped2.connected());
        EXPECT_TRUE(scoped1.connected());
        EXPECT_TRUE(c1.connected());
    }
    EXPECT_FALSE(c1.connected());
}