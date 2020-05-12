![Build status](https://github.com/jpvanoosten/signals/workflows/C/C++%20CI/badge.svg)

# Signals

A single header-only C++11 implementation of [boost::signals2] (okay... 2 header files). The implementation of this library is also heavly inspired by [palacaze/sigslot].

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
    s.connect(hello_world);

    // Call the signal.
    s();

    return 0;
}
```

The `hello_world` function is a free function that takes no arguments. It's only purpose is to print "Hello, World!" to the default output stream (the console).

In the the `main` function, a `signal` is aliased from `sig::signal<void()>`. `sig::signal` is a template class that takes the function signature as the template argument for the class. Creating a template alias for the signal is not required but it does make the code easier to read if you need to create multiple signals with the same function signatures.

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

The *combiner* class is a function object whose function call operator takes the `first` and `last` *input* iterators which invoke the slot when dereferenced. In this example, the `maximum_value` combiner is defined which iterators from `first` to `last` and invoking the slot by dereferencing the input iterator. The `result_type` type alias indicates to the signal the type of the return value of the combiner. In this case, the combiner returns an `opt::optional<T>`. If there are no slots connected to the signal, the result is a *disengaged* optional. If there is at least 1 unblocked slot connected to the signal, then the return value is the maximum value of all the connected slots.

Running the example should result in **15** being printed to the console.

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

The signal is invoked again outside of the scope block but in this case the `shared_ptr` instance goes out of scope and is destroyed. This causes the slot to become disconnected and the result of invoking the signal is a disengaged optional value.

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

    // Call the slot again.
    // Nothing is printed to the console.
    s();

    return 0;
}
```

In this example, a callable function object `HelloWorld` is connected to the signal using the `signal::connect` method. This method returns the `connection` object which is stored in `c`. The signal is invoked causing "Hello, World!" to be printed to the console. The slot is disconnected using the `connection::disconnect` method and the slot is invoked again but nothing is printed to the console.

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

Using the `connection_blocker` is just one method to block a slot from being invoked. The `connection::block` method and the `connection::unblock` methods can also be used to block and unblock the slot (respectively).

## Scoped Connections

The `connection` object does not automatically disconnect the slot whent it is destroyed. The `scoped_connection` object is used to automatically disconnect the slot when the `scoped_connection` object is destroyed. The `signal::connect_scoped` method is used to return a `scoped_connection` object.

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
Hello, World!
```


[jpvanoosten/signals]: https://github.com/jpvanoosten/signals
[sig::signals]: https://github.com/jpvanoosten/signals
[jpvanoosten/optional]: https://github.com/jpvanoosten/optional
[opt::optional]: https://github.com/jpvanoosten/optional
[boost::signals2]: https://www.boost.org/doc/libs/1_73_0/doc/html/signals2.html
[palacaze/sigslot]: https://github.com/palacaze/sigslot