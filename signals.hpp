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

#include <cstddef>
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

// since C++14
template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B,T>::type;

// since C++14
template<class T>
using decay_t = typename std::decay<T>::type;

// since C++17
template<class Base, class Derived>
inline constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

// Since C++17
template< class T >
inline constexpr bool is_member_function_pointer_v = std::is_member_function_pointer<T>::value;

// since C++17
template<class T>
inline constexpr bool is_function_v = std::is_function<T>::value;

// since C++17
// @see https://en.cppreference.com/w/cpp/types/void_t
template< typename... >
struct make_void
{
    typedef void type;
};

template<typename... T>
using void_t = typename make_void<T...>::type;

// Detect valid weak_ptr types.
// @see https://www.fluentcpp.com/2017/06/02/write-template-metaprogramming-expressively/
template<typename T, typename = void_t<>>
struct is_weak_ptr : std::false_type
{};

template<typename T>
struct is_weak_ptr< T, void_t< decltype(std::declval<T>().expired()),
                               decltype(std::declval<T>().lock()),
                               decltype(std::declval<T>().reset()) > >
       : std::true_type
{};

template<typename T>
inline constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

// Detect reference wrappers.
// @see https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper
template<typename T, typename = void_t<> >
struct is_reference_wrapper : std::false_type
{};

template<typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type
{};

template<typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

// Detect nullptr types.
// @see: https://en.cppreference.com/w/cpp/types/is_null_pointer
template<typename T>
struct is_null_pointer : std::is_same<std::nullptr_t, std::remove_cv<T>::type>
{};

template<typename T>
inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

// Get the result of an invokable function.
// @see https://en.cppreference.com/w/cpp/types/result_of
template<typename T>
struct invoke_impl
{
    // Deduce type of calling a (non member) function.
    template<typename Func, typename... Args>
    static auto call(Func&& f, Args&&... args) -> decltype(std::forward<Func>(f)(std::forward<Args>(args)...));
};

template<typename B, typename MT>
struct invoke_impl<MT B::*>
{
    template<typename T, typename Td = decay_t<T>,
        typename = enable_if_t<is_base_of_v<B, Td>>>
    static auto get(T&& t)  -> T&&;

    template<typename T, typename Td = decay_t<T>,
        typename = enable_if_t<is_reference_wrapper_v<Td>>>
    static auto get(T&& t) -> decltype(t.get());

    template<typename T, typename Td = decay_t<T>,
        typename = enable_if_t<!is_base_of_v<B, Td>,
        typename = enable_if_t<!is_reference_wrapper_v<Td>>
    static auto get(T&& t) -> decltype(*std::forward<T>(t));

    // Deduce the result of calling a pointer to member function.
    template<typename T, typename... Args, typename MT1,
        typename = enable_if_t<is_function_v<MT1>>
    static auto call(MT1 B::*pmf, T&& t, Args&&... args) -> decltype((invoke_impl::get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...));

    // Deduce the result of calling a pointer to data member.
    template<class T>
    static auto call(MT B::*pmd, T&& t) -> decltype(invoke_impl::get(std::forward<T>(t)).*pmd);
};

template<typename T, typename Type, typename T1, typename... Args>
constexpr auto INVOKE(Type T::* f, T1&& t1, Args&&... args) -> decltype(invoke_impl<decay_t<T>>::call(std::forward<Type>(f), std::forward<Args>(args)...))
{

}

template<typename F, typename... Args, class Fd = decay_t<F>>
constexpr auto INVOKE(F&& f, Args&&... args) -> decltype(invoke_impl<Fd>::call(std::forward<F>(f), std::forward<Args>(args)...))
{
    return std::forward<F>(f)(std::forward<Args>(args)...));
}

template<typename AlwaysVoid, typename, typename...>
struct invoke_result_impl
{};

template<typename F, typename... Args>
struct invoke_result_impl<decltype(void(INVOKE(std::declval<F>(), std::declval<Args>()...))), F, Args...>
{
    using type = decltype(INVOKE(std::declval<F>(), std::declval<Args>()...));
};

template<typename F, typename... Args>
struct invoke_result : invoke_result_impl<void, F, Args...>
{};

template<typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

} // namespace trait


} // namespace detail

/*
 * A slot object holds state information, and a callable to to be called
 * whenever the function call operator is called.
 */
template <typename Func, typename Ptr = std::nullptr_t>
class slot
{
public:
    slot() = delete;

    constexpr slot(Func&& f)
        : func{ std::forward<Func>(f) }
    {}

    constexpr slot(Ptr&& p, Func&& f)
        : ptr{ std::forward<Ptr>(p) }
        , func{ std::forward<Func>(f) }
    {}

    template <typename... Args,
    typename = detail::trait::enable_if_t<std::is_same<Ptr, std::nullptr_t>::value>>
     operator()(Args ...args) 
    {
        return func(args...);
    }

private:
    std::decay<Ptr>::type ptr;
    std::decay<Func>::type func;
};

}