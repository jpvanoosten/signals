#include "signals.hpp"
#include <iostream>

struct HelloWorld
{
    HelloWorld(int _id)
    : id(_id)
    {}

    void operator()() const
    {
        std::cout << "Hello, World!" << std::endl;
    }

    bool operator==(const HelloWorld& other) const
    {
        return id == other.id;
    }

private:
    int id;
};

int main()
{
    // Define a signal that takes no arguments and returns void.
    using signal = sig::signal<void()>;
    using slot = typename signal::slot_type;
    using connection = sig::connection;
    using connection_blocker = sig::connection_blocker;
    using scoped_connection = sig::scoped_connection;

    // Declare a signal.
    signal s;

    // Declare a slot
    slot sl = slot(HelloWorld(0));

    // Connect the slot directly to the signal.
    s.connect(sl);
    
    // Create a connection that has the same function signature
    // as the signal.
    connection c = s.connect(HelloWorld(1));
    
    // Create a scoped connection.
    scoped_connection sc(c);

    // Create a connection blocker.
    connection_blocker cb = c.blocker();

    // Invoke the signal.
    // "Hello, World!" is printed once (the slots's version).
    s();

    // Unblock the connection
    c.unblock();

    // Disconnect the slot.
    s.disconnect(sl);

    // Invoke the signal.
    // "Hello, World!" is printed once again.
    s();

    return 0;
}