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
    // Define a signal that takes two floats and returns a float.
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