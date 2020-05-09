#include "signals.hpp"
#include <iostream>

void hello_world()
{
    std::cout << "Hello, World!" << std::endl;
}

int main()
{
    // Defina a signal that takes no arguments and returns void.
    using signal = sig::signal<void()>;
    signal s;

    // Connect the hello_world function to the signal.
    s.connect(&hello_world);

    // Call the signal.
    s();

    return 0;
}