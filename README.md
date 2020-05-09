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
    // Defina a signal that takes no arguments and returns void.
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
    // Defina a signal that takes no arguments and returns void.
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



[jpvanoosten/signals]: https://github.com/jpvanoosten/signals
[sig::signals]: https://github.com/jpvanoosten/signals
[jpvanoosten/optional]: https://github.com/jpvanoosten/optional
[opt::optional]: https://github.com/jpvanoosten/optional
[boost::signals2]: https://www.boost.org/doc/libs/1_73_0/doc/html/signals2.html
[palacaze/sigslot]: https://github.com/palacaze/sigslot