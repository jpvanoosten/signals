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