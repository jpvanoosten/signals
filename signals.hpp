#pragma once
/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file signals.hpp
 *  @date March 24, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief The signals header file.
 *  @see https://www.boost.org/doc/libs/1_72_0/doc/html/signals2.html
 *  @see https://github.com/palacaze/sigslot
 */

#include <atomic>
#include <functional>
#include <type_traits>

namespace signals
{

namespace detail
{

namespace trait
{
/**
 * Represents a list of types.
 */
template < typename... >
struct typelist
{};

// from C++17
// @see https://en.cppreference.com/w/cpp/types/void_t
template< typename... >
struct make_void
{
    typedef void type;
};

template< typename... T>
using void_t = typename make_void<T...>::type;

// Detect valid weak_ptr types.
// @see https://www.fluentcpp.com/2017/06/02/write-template-metaprogramming-expressively/
template< typename T, typename = void_t<> >
struct is_weak_ptr : std::false_type
{};

template< typename T >
struct is_weak_ptr< T, void_t< decltype(std::declval<T>().expired()),
                               decltype(std::declval<T>().lock()),
                               decltype(std::declval<T>().reset()) > >
       : std::true_type
{};

template< typename T >
constexpr typename is_weak_ptr<T>::value_type is_weak_ptr_v = is_weak_ptr<T>::value;

} // namespace trait

/**
 * slot_state holds slot data that is independent of the slot type.
 */
class slot_state 
{
public:
    constexpr slot_state() noexcept
        : m_index(0)
        , m_connected(true)
        , m_blocked(false)
    {}

    virtual ~slot_state() = default;

    virtual bool connected() const noexcept
    {
        return m_connected;
    }

    bool disconnect() noexcept 
    {
        bool ret = m_connected.exchange(false);

        if (ret) 
        {
            do_disconnect();
        }

        return ret;
    }

    bool blocked() const noexcept 
    { 
        return m_blocked; 
    }

    void block()   noexcept 
    { 
        m_blocked = true; 
    }

    void unblock() noexcept 
    { 
        m_blocked = false; 
    }

protected:
    virtual void do_disconnect() 
    {}

    std::size_t& index() {
        return m_index;
    }

private:
    //template <typename, typename...>
    //friend class ::signals::signal_base;

    std::size_t m_index;  // index into the array of slot pointers inside the signal
    std::atomic_bool m_connected;
    std::atomic_bool m_blocked;
};

/**
 * Interface for cleanable objects, used to cleanup disconnected slots.
 */
struct cleanable {
    virtual ~cleanable() = default;
    virtual void clean(slot_state*) = 0;
};

/*
 * A slot object holds state information, and a callable to to be called
 * whenever the function call operator is called.
 */
template <typename WeakPtr, typename R, typename... Args>
class slot : public slot_state
{
public:
    using return_type = R;
    using args_type = typelist<Args...>;
    using func_type = std::function<R, Args...>;

    template <typename Func>
    constexpr slot(cleanable& c, Func&& f)
        : cleaner(c)
        , ptr{ nullptr }
        , func{ std::forward<F>(f) }
    {}

    template <typename Func, typename Ptr>
    constexpr slot(cleanable& c, Ptr&& p, Func&& f)
        : cleaner(c)
        , ptr{ std::forward<Ptr>(p) }
        , func{ std::forward<Func>(f) }
    {}

    virtual ~slot() override = default;

    template <typename... U>
    R operator()(U&& ...u) {
        if (slot_state::connected() && !slot_state::blocked()) {
            return call_slot(std::forward<U>(u)...);
        }
    }

protected:

    template<typename PtrType = WeakPtr, typename std::enable_if< is_weak_ptr< PtrType >::value >::type >
    R call_slot(Args... args)
    {
        return func(std::forward<Args>(args)...);
    }

private:
    template <typename, typename...>
    friend class ::signals::signal_base;

    std::decay<WeakPtr>::type ptr;
    std::function<R, Args...> func;
    cleanable& cleaner;
};



}

}