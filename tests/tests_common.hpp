#pragma once

#include <signals.hpp>
#include <iostream>

void void_func();
void void_func2();

void increment_counter(int& counter);


double difference(float i, float j);
double sum(float i, float j);
double product(float i, float j);
double quotient(float i, float j);

class VoidMemberFunc
{
public:
    VoidMemberFunc() = default;

    void DoSomething()
    {
        std::cout << "VoidMemberFunc::DoSomething" << std::endl;
    }
};

auto lambda = [](int i, int j) { return i + j; };
auto void_lambda = []() {};

struct Functor
{
    Functor(int i, int j)
        : m_i(i)
        , m_j(j)
    {}

    int operator()() const noexcept
    {
        return m_i + m_j;
    }

    int m_i, m_j;
};

class Base
{
public:
    Base(int i, int j)
        : m_i(i)
        , m_j(j)
    {}

    int multiply(int i, int j) const noexcept
    {
        return i * j;
    }

    virtual int sum() const noexcept
    {
        return m_i + m_j;
    }

protected:
    int m_i, m_j;
};

class Derived : public Base
{
public:
    Derived(int i, int j)
        : Base(i, j)
    {}

    virtual int sum() const noexcept override
    {
        return Base::sum() + 1;
    }
};

struct PointerToMemberData
{
    PointerToMemberData(int v)
        : value(v)
    {}

    int value;
};
