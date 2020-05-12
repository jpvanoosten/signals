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
    using slot = typename signal::slot_type;
    using connection = typename signal::connection_type;
    using connection_blocker = typename signal::connection_blocker_type;
    using scoped_connection = typename signal::scoped_connection_type;

    // Declare a signal.
    signal s;
    
    // Create a connection that has the same function signature
    // as the signal.
    connection c = s.connect(HelloWorld());
    
    // Create a scoped connection.
    scoped_connection sc(c);

    // Create a connection blocker.
    connection_blocker cb = c.blocker();

    // Invoke the signal.
    // Nothing is printed since the connection is blocked.
    s();

    // Unblock the connection
    c.unblock();

    // Invoke the signal.
    // "Hello, World!" is printed once again.
    s();

    return 0;
}