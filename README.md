![Build status](https://github.com/jpvanoosten/signals/workflows/C/C++%20CI/badge.svg)

# Signals

A single header-only C++11 implementation of [boost::signals2] (okay... 2 header files). The implementation of this library is also heavly inspired by [palacaze/sigslot].

## Usage

To use this library, just copy the [signals.hpp](signals.hpp) and [optional.hpp](optional.hpp) files to your include folder.

## Motivation

I needed a C++11 signals library that matched the functionality of the [boost::signals2] library without requiring the entire boost library to be included in the project. I've looked at a few other libraries that offered similar functionality but there were always trade-offs (for example, no support for return values, doesn't work with C++11, etc...).

This library provides a lot of the features of the [boost::signals2] library, without the need to include the entire boost library in your project. There are a few (I feel, seldom used) unimplemented features from the [boost::signals2] library (such as connection groups).

## Introduction

This is an implementation of a signals & slots library. There are three primary interfaces that make up this library:

- slot
- connection
- signal

// TODO: Add UML diagram.

### Slot

A `slot` is a holder for a function object. A function object can be anything that can be invoked. A few examples are:

- Free functions
- (Pointer to) member functions
- Function objects (Functors, or callable function objects)
- Lambda expressions
- (Pointer to) member data

For simplicity, any of these types of function-like objects will be generally referred to as *callable*.

A `slot` is used to hold any of these types of callable objects with any number (and types) of arguments and provide the return value of calling the slot (as an optional value).

The `slot` also stores its connection state. A *connected* `slot` is one that has a valid callable object associated with it. It is possible to invoke a disconnected `slot`. The return value of a disconnected `slot` is alwasy a *disengaged* [opt::optional] value. See [jpvanoosten/optional] for more information on the optional variable type used in this library. The return value of a connected `slot` should be an engaged [opt::optional] value.

### Connection

A `connection` object is used to manage the connection state of a slot within a signal. If a `connection` stores a reference to a valid and connected `slot`, it is in the connected state. The `connection` object can also be used to temporarily *block* a slot, *unblock* the slot, and disconnect the slot from the signal.

### Signal

The `signal` class is probably the most common way of working with the slots & signals. The `signal` is a container for multiple `slots`. The `signal` can be invoked which results in all of the connected slots being invoked.

## Hello, World!

The simplest example of using the signals & slots library is to create a signal with a single slot that calls a free function that prints "Hello, World!" to the console.

```cpp
#include "signals.hpp"
#include <iostream>

void hello_world()
{
    std::cout << "Hello, World!" << std::endl;
}

int main()
{
    // Define a signal that takes no arguments and returns void.
    using signal = sig::signal<void()>;
    signal s;

    // Connect the hello_world function to the signal.
    s.connect(&hello_world);

    // Call the signal.
    s();

    return 0;
}
```

The `hello_world` function is a free function that takes no arguments. It's only purpose is to print "Hello, World!" to the default output stream (the console).

In the the `main` function, a `signal` is aliased from `sig::signal<void()>`. `sig::signal` is a template class that takes the function signature as the template argument for the class. Creating a template alias for the signal is not required but it does make the code easier to read if you need to create multiple signals with the same function signature.

The `hello_world` function is connected to the signal using the `signal::conect` method.

The signal is invoked by using the function call operator `s()`.

When you run the program, you should see "Hello, World!" printed in the console.

```sh
Hello, World!
```

## Connecting Multiple Slots

Multiple slots can be connected to a signal. In this case, the task of printing "Hello, World!" to the console is split into separate functions.

```cpp
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
```

In this example, the functions `hello` and `world` are connected to the same signal. When the signal is invoked, the text "Hello, World!" is printed to the console.

```sh
Hello, World!
```

## Slot Arguments

A slot can hold functions that takes multiple arguments.

```cpp
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
    // Define a signal that takes two floats and returns void.
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
```

In this example, five slots are connected to the signal. Each slot takes two floating point values as arguments and returns void. Each slot that is connected to the signal must match the signature of the signal.

The output of running this example should be:

```sh
The arguments are 5 and 3
The sum is 8
The product is 15
The difference is 2
The quotient is 1.66667
```

## Signal Return Values

Slots can also return values. If multiple slots are connected to a signal then the result of invoking the signal is determined by the a *combiner* that is associated with the signal. The default combiner is `sig::optional_last_value` which returns the result of the last slot that is connected to the signal.

```cpp
#include "signals.hpp"
#include <iostream>

float product(float x, float y) { return x * y; };
float quotient(float x, float y) { return x / y; }
float sum(float x, float y) { return x + y; }
float difference(float x, float y) { return x - y; }

int main()
{
    // Define a signal that takes two floats and returns a float.
    using signal = sig::signal<float(float, float)>;
    signal s;

    // Connect all of the functions to the signal.
    s.connect(&product);
    s.connect(&quotient);
    s.connect(&sum);
    s.connect(&difference);

    // The default combiner returns a opt::optional containing
    // the return value of the last slot in the list, in this case
    // the result of the difference function.
    std::cout << *s(5.0f, 3.0f) << std::endl;

    return 0;
}
```

In this example, four functions are connected to the signal. When the signal is invoked, all of the functions are invoked but ony the result of the last slot (`difference`) is returned. If you run this program, the following output should be printed to the console:

```sh
2
```

It is possible to override the default combiner for the slot by supplying a combiner class as one of the template arguments for the slot.

```cpp
#include "signals.hpp"
#include <iostream>

template<typename T>
class maximum_value
{
public:
    using result_type = opt::optional<T>;

    template<typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const
    {
        result_type max;
        while (first != last)
        {
            // Dereferencing the iterator invokes the connected function.
            result_type tmp = *first;
            if ( tmp > max) max = tmp;

            ++first;
        }
        return max;
    }
};

float product(float x, float y) { return x * y; };
float quotient(float x, float y) { return x / y; }
float sum(float x, float y) { return x + y; }
float difference(float x, float y) { return x - y; }

int main()
{
    // Define a signal that takes two floats and returns void.
    // The return value of the signal is the maximum value of all connected slots.
    using signal = sig::signal<float(float, float), maximum_value<float>>;
    signal s;

    // Connect all of the functions to the signal.
    s.connect(&product);
    s.connect(&quotient);
    s.connect(&sum);
    s.connect(&difference);

    // The maximum_value combiner returns the maximum
    // value returned by all connected slots.
    // In this case, the result is 15 since 5 * 3 is 15.
    std::cout << *s(5.0f, 3.0f) << std::endl;

    return 0;
}
```

The *combiner* class is a function object whose function call operator takes the `first` and `last` *input* iterators which invoke the slot when dereferenced. In this example, the `maximum_value` combiner is defined which iterates from `first` to `last` and invoking the slot by dereferencing the iterator. The `result_type` type alias indicates to the signal the type of the return value of the combiner. In this case, the combiner returns an `opt::optional<T>`. If there are no slots connected to the signal, the result is a *disengaged* optional value. Otherwise, the return value is the maximum value of all the connected slots.

Running the example should result in 15 being printed to the console.

```sh
15
```

As another example, we may want to return a list (`std::vector`) of all of the return values of all of the connected slots.

```cpp
#include "signals.hpp"
#include <iostream>
#include <vector>

template<typename Container>
class aggregate_values
{
public:
    using result_type = Container;

    template<typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const
    {
        result_type values;
        while (first != last)
        {
            auto value = *first++;
            if ( value )
                values.push_back(*value);
        }
        return values;
    }
};

float product(float x, float y) { return x * y; };
float quotient(float x, float y) { return x / y; }
float sum(float x, float y) { return x + y; }
float difference(float x, float y) { return x - y; }

int main()
{
    // Define a signal that takes two floats and returns a float.
    // The return value of the signal is a list of the return values of all connected slots.
    using signal = sig::signal<float(float, float), aggregate_values<std::vector<float>>>;
    signal s;

    // Connect all of the functions to the signal.
    s.connect(&product);
    s.connect(&quotient);
    s.connect(&sum);
    s.connect(&difference);

    std::cout << "Aggregate values: ";
    // The aggregate_values combiner returns a list
    // of all of the values from the connected slots.
    for ( auto f : s(5.0f, 3.0f) )
    {
        std::cout << f << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

In this example, the *combiner* for the signal returns a list of return values from all connected slots in an `std::vector`. For the combiner in this example, any container type that provides the `push_back` method can be used.

The result of running this example should be:

```sh
Aggregate values: 15 1.66667 8 2
```

## Member Functions

Connecting a signal to a member function of an instance of a class is simply a matter of passing a pointer to the class instance as the second parameter of the `signal::connect` method.

```cpp
#include "signals.hpp"
#include <iostream>

class Calculator
{
public:
    float product(float x, float y)
    {
        return x * y;
    }

    float sum(float x, float y)
    {
        return x + y;
    }

    float difference(float x, float y)
    {
        return x - y;
    }

    float quotient(float x, float y)
    {
        return x / y;
    }
};

int main()
{
    // Define a signal that takes two floats and returns a float.
    // The return value of the signal is a list of all return values of connected slots.
    using signal = sig::signal<float(float, float)>;
    signal s;

    // Create an instance of calculator.
    Calculator c;

    // Connect all of the functions to the signal.
    s.connect(&Calculator::product, &c);
    s.connect(&Calculator::sum, &c);
    s.connect(&Calculator::quotient, &c);
    s.connect(&Calculator::difference, &c);

    std::cout << *s(5.0f, 3.0f) << std::endl;

    return 0;
}
```

In this example, the `Calculator` class defines four methods. Each method is connected to the signal passing an instance of the class as the second parameter to the `signal::connect` method. Similar to the previous example, since the `Calculator::difference` method is connected last, the result of invoking the signal is 2.

```sh
2
```

It is important to note that the signal does not (cannot) track the lifetime of the instance of the object that is passed as the second parameter of the `connect` method. In this case, the signal must not be invoked on a class instance that goes out of scope.

```cpp
...
using signal = sig::signal<float(float, float)>;
signal s;

{
    // Create an instance of calculator.
    Calculator c;

    // Connect all of the functions to the signal.
    s.connect(&Calculator::product, &c);
    s.connect(&Calculator::sum, &c);
    s.connect(&Calculator::quotient, &c);
    s.connect(&Calculator::difference, &c);
}   // Oops.. c is out of scope and has been destroyed.

// Invoking the slot now will fail since c is out of scope!
std::cout << *s(5.0f, 3.0f) << std::endl;
...
```

In the example shown above, an instance of the `Calculator` class is created and connected to the signal inside a scope block. The signal is invoked outside of that scope block which means that the instance of the `Calculator` class has been destroyed and very likely a rutime error will occur.

To solve this problem, you can connect a *shared pointer* with the signal.

## Automatic Connection Management

Slots can automatically manage their connection state if the pointer to the instance variable is a *shared pointer* object (or anything that is convertable to a `std::weak_ptr`).

```cpp
#include "signals.hpp"
#include <iostream>
#include <memory>

class Calculator
{
public:
    float product(float x, float y) { return x * y; }
    float sum(float x, float y) { return x + y; }
    float difference(float x, float y) { return x - y; }
    float quotient(float x, float y) { return x / y; }
};

int main()
{
    // Define a signal that takes two floats and returns a float.
    // The return value of the signal is a list of all return values of connected slots.
    using signal = sig::signal<float(float, float)>;
    signal s;

    {    // Create a shared pointer instance of Calculator.
        auto c = std::make_shared<Calculator>();

        // Connect all of the functions to the signal
        // with a shared pointer to the calculator instance.
        s.connect(&Calculator::product, c);
        s.connect(&Calculator::sum, c);
        s.connect(&Calculator::quotient, c);
        s.connect(&Calculator::difference, c);

        // Okay, should return 2.
        std::cout << *s(5.0f, 3.0f) << std::endl;
    } // The shared pointer instance goes out of scope and should be destroyed.

    // Invoking the signal again should result in a disengaged optional value.
    auto result = s(5.0f, 3.0f);
    if ( result )
    {
        std::cout << *result << std::endl;
    }
    else
    {
        std::cout << "Invalid result!" << std::endl;
    }

    return 0;
}
```

In this example, a `shared_ptr` instance of the `Calculator` class is created inside the scope block and connected to the signal. When the signal is invoked inside the scope block, it returns the value 2.

The signal is invoked again outside of the scope block but in this case the `shared_ptr` instance goes out of scope and is destroyed. This causes the slots to become disconnected and the result of invoking the signal is a disengaged optional value.

The result of executing this example is:

```sh
2
Invalid result!
```

But it would be pretty frustrating if this was the only way to disconnect a slot from a signal. In the following sections, several different methods of connection management are described.

## Disconnecting Slots

The return value of the `signal::connect` method is a `connection` object. The `connection` object is used to disconnect the slot from the signal.

```cpp
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

    // Call the signal again.
    // Nothing is printed to the console.
    s();

    return 0;
}
```

In this example, a callable function object `HelloWorld` is connected to the signal using the `signal::connect` method. This method returns the `connection` object which is stored in `c`. The signal is invoked causing "Hello, World!" to be printed to the console. The slot is disconnected using the `connection::disconnect` method and the signal is invoked again but nothing is printed to the console.

```sh
Hello, World!
```

## Blocking Slots

Slots can be temporarly blocked by using a `connection_blocker` object. A `connection_bloker` can be obtained from the original `connection` object using the `connection::blocker` method.

```cpp
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
```

The `HelloWorld` callable function object is connected to the signal and the resulting `connection` object is stored in `c`.

The signal is invoked which causes "Hello, World!" to be printed to the screen.

Inside the scope block, a `connection_blocker` is created and stored in the varible `b`. The signal is invoked again inside the scope block, but nothing is printed to the screen since the slot is blocked. When the `connection_blocker` goes out of scope, it unblocks the slot.

Outside of the scope block, the signal is invoked again, causing "Hello, World!" to be printed to the console again.

```sh
Hello, World!
Hello, World!
```

Using the `connection_blocker` is just one method to block a slot from being invoked. The `connection::block` method and the `connection::unblock` method can also be used to block and unblock the slot (respectively).

## Scoped Connections

The `connection` object does not automatically disconnect the slot from the signal when it is destroyed. The `scoped_connection` object can be used to automatically disconnect the slot when the `scoped_connection` object is destroyed. The `signal::connect_scoped` method is used to return a `scoped_connection` object.

```cpp
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
```

In this example, a slot is connected to the signal using the `signal::connect_scoped` method which returns a `scoped_connection` object and is stored in `sc`.

Inside the scope block, the signal is invoked causing "Hello, World!" to be printed to the console.

When `sc` goes out of scope, it automatically disconnects the slot from the signal and when the signal is invoked again outside of the scope block, nothing is printed to the screen.

```sh
Hello, World!
```

## Disconnecting Equivalent Slots

Similar to `signal::connect`, slots can be disconnected from the signal using `signal::disconnect` and passing the callable function object as a parameter. If the callable function object can be matched to an existing slot, it will be removed from the signal.

The function object must be equality comparable (that is, it must define the equality operator `==`). If the function objects are not compareable, a `sig::not_comparable_exception` exception is thrown.

```cpp
#include "signals.hpp"
#include <iostream>

float product(float x, float y) { return x * y; };
float quotient(float x, float y) { return x / y; }
float sum(float x, float y) { return x + y; }
float difference(float x, float y) { return x - y; }

int main()
{
    // Define a signal that takes two floats and returns a float.
    using signal = sig::signal<float(float,float)>;
    signal s;

    // Connect all of the functions to the signal.
    s.connect(&product);
    s.connect(&quotient);
    s.connect(&sum);
    s.connect(&difference);

    // Should print 2 (the result of difference)
    std::cout << *s(5.0f, 3.0f) << std::endl;

    // Disconnect the last slot.
    s.disconnect(&difference);

    // Should print 8 (the result of sum)
    std::cout << *s(5.0f, 3.0f) << std::endl;

    // Disconnect the first slot.
    // For efficency, sum becomes the first slot.
    s.disconnect(&product);

    // This actually prints 1.6667 since
    // quotient becomes the last slot in the signal.
    std::cout << *s(5.0f, 3.0f) << std::endl;

    s.disconnect(&quotient);

    // Should now print 8 (the result of sum)
    std::cout << *s(5.0f, 3.0f) << std::endl;

    s.disconnect(&sum);

    // All slots disconnected.
    // Invoking the signal now should result
    // in a disengaged opt::optional value.
    auto result = s(5.0f, 3.0f);
    if ( result )
    {
        std::cout << *result << std::endl;
    }
    else
    {
        std::cout << "Result is invalid!" << std::endl;
    }

    return 0;
}
```

In this example, a few free functions are defined. The functions are connected to the signal and the signal is invoked. The result of invoking the signal is 2 (the result from `difference`);

Then the last slot is disconnected and the signal is invoked again. This time, the result is 8 (the result from `sum`).

Then the `product` slot is disconnected and the signal is invoked again. It is interesting to note that the result that is printed to the console is from `quotient` despite the fact that `sum` should be the last slot. This occurs because when `product` is removed, `product` is swapped with the last slot and then `product` is removed from the end of the container. Since the signal uses an `std::vector` for its internal container, the slots are swapped to avoid having to relocate all of the slots that appear after the slot being removed. This makes the remove *constant-time* instead of *linear* in the number of slots being relocated. It is important to be aware of the slot reordering when removing slots from the beginning or middle of the container.

Then the `quotient` slot is removed and the signal is invoked again, printing 8 to the console (the result of `sum`).

Finally, `sum` is removed from the signal and the signal is invoked again. Since there are no slots left, the result of invoking the signal is a disengaged `opt::optional` value.

```sh
2
8
1.66667
8
Result is invalid!
```

## Signal Type Aliases

`sig::signal` is a template class that takes the function signature as its first template argument. For convienience, the `signal` class defines type aliases for matching `slot`, `connection`, `connection_blocker`, and `scoped_connection` objects.

```cpp
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
    using slot = signal::slot_type;
    using connection = signal::connection_type;
    using connection_blocker = signal::connection_blocker_type;
    using scoped_connection = signal::scoped_connection_type;

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
```

For convienience, the `signal` defines several type aliases for the `slot`, `connection`, `connection_blocker`, and `scoped_connection` types that have the same function signature as the signal.

## Event Delegates

Using the `sig::signal` library, it is easy to create an event system that is similar to the C# event system.

### Delegate Class

The `Delegate` class defines a set of callback functions. For simplicity of the example, only functions returning `void` are allowed but any number of arguments can be used to define a delegate.

```cpp
#include "signals.hpp"
#include <iostream>

// Delegate that holds function callbacks.
// For simplicity, delegates are limited to void return types.
template<typename... Args>
class Delegate
{
public:
    using signal = sig::signal<void(Args...)>;
    using connection = typename signal::connection_type;

    // Adds a function callback to the delegate.
    template<typename Func>
    connection operator+=(Func&& f)
    {
        return m_Callbacks.connect(std::forward<Func>(f));
    }

    // Remove a function callback.
    // Returns the number of functions removed.
    template<typename Func>
    std::size_t operator-=(Func&& f)
    {
        return m_Callbacks.disconnect(std::forward<Func>(f));
    }

    // Invoke the delegate.
    // All connected callbacks are invoked.
    void operator()(Args&&... args)
    {
        m_Callbacks(std::forward<Args>(args)...);
    }
private:
    signal m_Callbacks;
};
```

The `Delegate` class is a wrapper for a `sig::signal` but provides `+=` and `-=` operators for connecting and disconnecting slots to the underlying `signal` (similar to C# delegates). The delegate also defines a function call operator to invoke the registered callbacks.

### Event

With the `Delegate` class defined, a few `Events` can be defined that take an `EventArgs` as the only argument to the event callback:

```cpp
// Base class for all event args.
class EventArgs
{
public:
    EventArgs() = default;
    virtual ~EventArgs() = default;
};

// Define an event that takes a reference to EventArgs
// as it's only argument.
using Event = Delegate<EventArgs&>;
```

The `Event` object can be used to register callbacks that takes a reference to an `EventArgs` as the only argument.

### Mouse Motion Event

The `Event` can be used for generic callback functions, but when you need to pass more information to the callback, you can pass those arguments through the event args. For example, the `MouseMotionEventArgs` stores the x, and y coordinates of the mouse when the event is triggered.

```cpp
class MouseMotionEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseMotionEventArgs( int x, int y )
        : base()
        , X(x)
        , Y(y)
    {}

    int X;
    int Y;
};

// Define an event that is fired when the mouse moves over
// the application window.
using MouseMotionEvent = Delegate<MouseMotionEventArgs&>;
```

The `MouseMotionEventArgs` stores the x, and y coordinates of the mouse when the event is fired. The `MouseMotionEvent` is an event delegate that accepts callback functions that take a reference to a `MouseMotionEventArgs` as the only argument.

### Application Class

With a few events defined, an `Application` class can be created that exposes a few events that the client can register callback functions for.

```cpp
// The Application class defines a few events that can be handled by
// callback functions elsewhere.
class Application
{
public:
    Application() = default;
    virtual ~Application() = default;

    // Application events.
    Event            Update;
    Event            Render;
    MouseMotionEvent MouseMoved;

protected:
    // Give the WndProc method access to the
    // protected members of this class.
    friend void WndProc();

    virtual void OnUpdate()
    {
        EventArgs e;
        // Invoke event.
        Update(e);
    }

    virtual void OnRender()
    {
        EventArgs e;
        // Invoke event.
        Render(e);
    }

    virtual void OnMouseMoved(int x, int y)
    {
        MouseMotionEventArgs e(x, y);
        // Invoke event.
        MouseMoved(e);
    }
};
```

The `Application` class exposes 3 events:

- Update
- Render
- MouseMoved

The client application can register callback functions that are invoked when those events are fired by the application.

As a simple example, let's suppose that the windows process callback function looks like this:

```cpp
// I know, globals are evil, but it makes the
// example easier.
static Application app;
// Simulate a windows process function.
void WndProc()
{
    app.OnUpdate();
    app.OnRender();
    app.OnMouseMoved(60, 80);
}
```

### Client Code

The client code can register callback functions that are invoked when the events are fired by the application. First, a few callback functions are defined.

```cpp
// Define a few callback functions that will handle events from the application.
void OnUpdate(EventArgs& e)
{
    std::cout << "Update game..." << std::endl;
}

void OnRender(EventArgs& e)
{
    std::cout << "Render game..." << std::endl;
}

void OnMouseMoved(MouseMotionEventArgs& e)
{
    std::cout << "Mouse moved: " << e.X << ", " << e.Y << std::endl;
}
```

Then, the callback functions can be registered with the applications events:

```cpp
int main()
{
    // Register some callback functions.
    app.Update += &OnUpdate;
    app.Render += &OnRender;
    app.MouseMoved += &OnMouseMoved;

    // Execute the windows event processor
    WndProc();

    // Unregister callback functions
    app.Update -= &OnUpdate;
    app.Render -= &OnRender;
    app.MouseMoved -= &OnMouseMoved;

    // Execute the windows event processor again.
    // This time, nothing should happen.
    WndProc();

    return 0;
}
```

First, the callback functions are registered with the applications events.

The `WndProc` function is invoked which simulates the windows processor function. The `WndProc` function just invokes the applications events which causes the callback functions to be invoked and the following is printed to the console:

```sh
Update game...
Render game...
Mouse moved: 60, 80
```

Next, the callback functions are unregistered from the application's events and the `WndProc` function is called again. This time, nothing is printed to the console.

## Conclusion

The `sig::signal` library is a C++11 single-header (okay 2 header) library that provides a signal & slot implementation.

## Known Issues

1. `sig::signal` does not support connection groups (similar to [boost::signals2]).
2. `sig::signal` does not support extended connections (in boost, this is `signal::connect_extended`).
3. Currently, the `sig::detail::slot_iterator` class is used to iterate slots in a `Combiner`. The iterator should automatically skip blocked or disconnected slots but these are still invoked when the iterator is dereferenced resulting in a disengaged optional value being retured from the slot. Ideally, blocked or disconnected slots should be skipped when the iterator is incremented (using either pre or post-increment operator).
4. When the `sig::detail::slot_iterator` is dereferenced in the `Combiner`, the result of invoking the slot is not cached. This means that dereferencing the iterator in the combiner several times will invoke the slot each time which could potentially be an expensive operation or even change the result that is returned from the slot (if invoking the slot has side-effects). Ideally, the result of invoking the slot should be cached until the iterator is incremented to the next slot.

[jpvanoosten/signals]: https://github.com/jpvanoosten/signals
[sig::signals]: https://github.com/jpvanoosten/signals
[jpvanoosten/optional]: https://github.com/jpvanoosten/optional
[opt::optional]: https://github.com/jpvanoosten/optional
[boost::signals2]: https://www.boost.org/doc/libs/1_73_0/doc/html/signals2.html
[palacaze/sigslot]: https://github.com/palacaze/sigslot
