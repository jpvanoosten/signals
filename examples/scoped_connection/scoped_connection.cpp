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

    {
        // Create scoped_connection object.
        // A scoped_connection will disconnect the signal
        // when it goes out of scope.
        auto sc = s.connect_scoped(HelloWorld());

        // Invoke the signal again.
        // This should print "Hello, World!" to the console.
        s();
    }

    // Invoke the slot again.
    // Nothing is printed to the console.
    s();

    return 0;
}