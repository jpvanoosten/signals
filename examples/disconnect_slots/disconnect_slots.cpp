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