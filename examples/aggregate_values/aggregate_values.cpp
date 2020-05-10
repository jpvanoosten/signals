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
    // The return value of the signal is a list of all return values of connected slots.
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
    for ( auto f : s(5.0f, 3.0f ))
    {
        std::cout << f << " ";
    }
    std::cout << std::endl;

    return 0;
}