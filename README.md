![Build status](https://github.com/jpvanoosten/signals/workflows/C/C++%20CI/badge.svg)

# Signals

A single header-only C++11 implementation of [boost::signals2] (okay... 2 header files).

## Motivation

I needed a signals library that matched the functionality of the [boost::signals2] library without requiring the entire boost library to be included in the project. I've looked at a few other libraries that offered similar functionality but there were always trade-offs (for example, no support for return values).

This library provides a lot of the features of the [boost::signals2] library, without the need to include the entire boost library in your project.

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

A `slot` is used to hold any of these types of objects with any number (and types) of arguments and provide the return value of calling the slot (as an optional value).

The `slot` also stores its connection state. A *connected* `slot` is one that has a valid callable object associated with it. It is possible to invoke a disconnected `slot`. The return value of a disconnected `slot` is alwasy a *disengaged* [opt::optional] value. See [jpvanoosten/optional] for more information on the optional variable type used in this library. The return value of a connected `slot` should be an engaged [opt::optional] value.

### Connection

A `connection` object is used to manage the connection state of a slot within a signal. If a `connection` stores a reference to a valid `slot`, it is in the connected state. The `connection` object can also be used to temporarily *block* a slot, *unblock* the slot, and disconnect the slot from the signal.

### Signal

The `signal` class is probably the most common way of working with the slots & signals. The `signal` is a container for multiple `slots`. The `signal` can be invoked which results in all of the connected slots being invoked.


[jpvanoosten/signals]: https://github.com/jpvanoosten/signals
[sig::signals]: https://github.com/jpvanoosten/signals
[jpvanoosten/optional]: https://github.com/jpvanoosten/optional
[opt::optional]: https://github.com/jpvanoosten/optional
[boost::signals2]: https://www.boost.org/doc/libs/1_73_0/doc/html/signals2.html