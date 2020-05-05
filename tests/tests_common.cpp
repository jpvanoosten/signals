#include "tests_common.hpp"
#include <iostream>

void void_func()
{
    std::cout << "void_func()" << std::endl;
}

void void_func2()
{
    std::cout << "void_func2()" << std::endl;
}

void increment_counter(int& counter)
{
    ++counter;
}

// Function takes default arguments.
void default_arguments(int& i, int c)
{
    i += c;
}

double sum(float i, float j)
{
    return static_cast<double>(i) + static_cast<double>(j);
}

double difference(float i, float j)
{
    return static_cast<double>(i) - static_cast<double>(j);
}

double product(float i, float j)
{
    return static_cast<double>(i) * static_cast<double>(j);
}

double quotient(float i, float j)
{
    return static_cast<double>(i) / static_cast<double>(j);
}



