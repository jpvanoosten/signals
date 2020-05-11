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