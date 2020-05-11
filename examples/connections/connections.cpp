#include "signals.hpp"
#include <iostream>

struct HelloWorld
{
    void operator()() const
    {
        std::cout << "Hello, World!" << std::endl;
    }
};

int main()
{
    // Define a signal that takes no arguments and returns void.
    using signal = sig::signal<void()>;
    signal s;

    // Connect the HelloWorld function object to the signal.
    auto c = s.connect(HelloWorld());

    // Call the signal.
    // This should print "Hello, World!" to the console.
    s();

    // Disconnect the slot.
    c.disconnect();

    // Call the slot again.
    // Nothing is printed to the console.
    s();

    return 0;
}