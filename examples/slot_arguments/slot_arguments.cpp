#include "signals.hpp"
#include <iostream>

void print_args(float x, float y)
{
    std::cout << "The arguments are " << x << " and " << y << std::endl;
}

void print_sum(float x, float y)
{
    std::cout << "The sum is " << x + y << std::endl;
}

void print_product(float x, float y)
{
    std::cout << "The product is " << x * y << std::endl;
}

void print_difference(float x, float y)
{
    std::cout << "The difference is " << x - y << std::endl;
}

void print_quotient( float x, float y)
{
    std::cout << "The quotient is " << x / y << std::endl;
}

int main()
{
    // Defina a signal that takes two floats and returns void.
    using signal = sig::signal<void(float, float)>;
    signal s;

    // Connect the hello_world function to the signal.
    s.connect(&print_args);
    s.connect(&print_sum);
    s.connect(&print_product);
    s.connect(&print_difference);
    s.connect(&print_quotient);

    // Call the signal.
    s(5.0f, 3.0f);

    return 0;
}