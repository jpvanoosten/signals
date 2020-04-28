/**
 * Tests the copy-on-write pointer type defined in signals.hpp.
 */

#include <signals.hpp>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

TEST(cow, BasicTests)
{
    using string_ptr = sig::detail::cow_ptr<std::string>;

    string_ptr sp1("Hello world!");
    string_ptr sp2(sp1);

    EXPECT_EQ(sp1, sp2);

    std::cerr << *sp1 << std::endl;
    std::cerr << *sp2 << std::endl;

    *sp2 = "Goodbye cruel world!";

    std::cerr << *sp1 << std::endl;
    std::cerr << *sp2 << std::endl;

    EXPECT_NE(sp1, sp2);
}