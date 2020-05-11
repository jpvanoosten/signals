#include "signals.hpp"
#include <iostream>

void hello()
{
    std::cout << "Hello";
}

void world()
{
    std::cout << ", World!" << std::endl;
}

int main()
{
    // Define a signal that takes no arguments and returns void.
    using signal = sig::signal<void()>;
    signal s;

    // Connect the hello, and world functions to the signal.
    s.connect(&hello);
    s.connect(&world);

    // Call the signal.
    s();

    return 0;
}