#include "signals.hpp"
#include <iostream>

float product(float x, float y) { return x * y; };
float quotient(float x, float y) { return x / y; }
float sum(float x, float y) { return x + y; }
float difference(float x, float y) { return x - y; }

int main()
{
    // Defina a signal that takes two floats and returns a float.
    using signal = sig::signal<float(float, float)>;
    signal s;

    // Connect all of the functions to the signal.
    s.connect(&product);
    s.connect(&quotient);
    s.connect(&sum);
    s.connect(&difference);

    // The default combiner returns an opt::optional containing
    // the return value of the last slot in the list, in this case
    // the result of the difference function.
    std::cout << *s(5.0f, 3.0f) << std::endl;

    return 0;
}