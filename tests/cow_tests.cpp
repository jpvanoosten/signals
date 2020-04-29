/**
 * Tests the copy-on-write (cow) pointer type defined in signals.hpp.
 */

#include <signals.hpp>
#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

using sig::detail::cow_ptr;
using sig::detail::make_cow;

TEST(cow, BasicTests)
{
    auto sp1 = make_cow<std::string>("Hello world!");
    auto sp2(sp1);

    EXPECT_EQ(sp1, sp2);
    EXPECT_EQ(sp1.read(), "Hello world!");
    EXPECT_EQ(sp2.read(), "Hello world!");
    EXPECT_EQ(sp1, sp2);

    *sp2 = "Goodbye cruel world!";

    EXPECT_NE(sp1, sp2);
    EXPECT_EQ(sp1.read(), "Hello world!");
    EXPECT_EQ(sp2.read(), "Goodbye cruel world!");
}

TEST(cow, CowVector)
{
    using vector_type = std::vector<int>;

    auto vp1 = make_cow<vector_type>({0, 1, 2, 3, 4});

    auto vp2(vp1);
    auto vp3(vp2);

    EXPECT_EQ(vp1, vp2);
    EXPECT_EQ(vp1, vp3);
    EXPECT_EQ(vp1.read(), vp2.read());
    EXPECT_EQ(vp1, vp2);

    // Add a value to vp2.
    vp2->push_back(5);

    // Now vp1 and vp2 will differ.
    EXPECT_NE(vp1, vp2);
    // But vp1 and vp3 are still pointing to the same object.
    EXPECT_EQ(vp1, vp3);
}