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

    // Invoke the signal.
    // This should print "Hello, World!" to the console.
    s();

    {
        // Create a connection_blocker which will block the slot until
        // the connection_blocker is destroyed.
        // You can also use c.block() but then you need to call 
        // c.unblock() to unblock the slot again.
        auto b = c.blocker();

        // Invoke the signal again.
        // In this case, nothing is printed to the console.
        s();
    }

    // Invoke the slot again.
    // This should print "Hello, World!" to the console.
    s();

    return 0;
}