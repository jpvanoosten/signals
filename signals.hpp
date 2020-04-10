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

namespace sig
{
    namespace detail
    {
        namespace traits
        {
            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/enable_if
            template<bool B, class T = void>
            using enable_if_t = typename std::enable_if<B, T>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/decay
            template<class T>
            using decay_t = typename std::decay<T>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/conditional
            template<bool B, class T, class F>
            using conditional_t = typename std::conditional<B, T, F>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/remove_cv
            template< class T >
            using remove_cv_t = typename std::remove_cv<T>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/remove_cv
            template< class T >
            using remove_const_t = typename std::remove_const<T>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/remove_cv
            template< class T >
            using remove_volatile_t = typename std::remove_volatile<T>::type;

            // since C++14
            // @see https://en.cppreference.com/w/cpp/types/remove_reference
            template< class T >
            using remove_reference_t = typename std::remove_reference<T>::type;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/integral_constant
            template <bool B>
            using bool_constant = std::integral_constant<bool, B>;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_base_of
            template<class Base, class Derived>
            constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_member_function_pointer
            template< class T >
            constexpr bool is_member_function_pointer_v = std::is_member_function_pointer<T>::value;

            // since C++17
            // @see 
            template< class T >
            constexpr bool is_member_object_pointer_v = std::is_member_object_pointer<T>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_function
            template<class T>
            constexpr bool is_function_v = std::is_function<T>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/void_t
            template< class... >
            struct make_void
            {
                typedef void type;
            };

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/void_t
            template<class... T>
            using void_t = typename make_void<T...>::type;

            // since C++17
            template< class T >
            constexpr bool is_void_v = std::is_void<T>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/conjunction
            template<class...> struct conjunction : std::true_type {};
            template<class B1> struct conjunction<B1> : B1 { };
            template<class B1, class... Bn>
            struct conjunction<B1, Bn...> : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/conjunction
            template<class... B>
            constexpr bool conjunction_v = conjunction<B...>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/disjunction
            template<class...> struct disjunction : std::false_type {};
            template<class B1> struct disjunction<B1> : B1 {};
            template<class B1, class... Bn>
            struct disjunction<B1, Bn...> : conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/disjunction
            template<class... B>
            constexpr bool disjunction_v = disjunction<B...>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_convertible
            template< class From, class To >
            constexpr bool is_convertible_v = std::is_convertible<From, To>::value;

            template<class To>
            void implicitly_convert_to(To) noexcept; // not defined

            // since C++20
            // @see https://en.cppreference.com/w/cpp/types/is_convertible
            template <class From, class To, bool = is_convertible_v<From, To>, bool = is_void_v<To>>
            constexpr bool is_nothrow_convertible_v = noexcept(implicitly_convert_to<To>(std::declval<From>()));

            template <class From, class To, bool IsVoid>
            constexpr bool is_nothrow_convertible_v<From, To, false, IsVoid> = false;

            template <class From, class To>
            constexpr bool is_nothrow_convertible_v<From, To, true, true> = true;

            // since C++20
            // @see https://en.cppreference.com/w/cpp/types/is_convertible
            template <class From, class To>
            struct is_nothrow_convertible : bool_constant<is_nothrow_convertible_v<From, To>>
            {};

            // since C++20
            // @see https://en.cppreference.com/w/cpp/types/remove_cvref
            template<class T>
            struct remove_cvref {
                typedef remove_cv_t<remove_reference_t<T>> type;
            };

            // since C++20
            // @see https://en.cppreference.com/w/cpp/types/remove_cvref
            template<class T>
            using remove_cvref_t = typename remove_cvref<T>::type;

            // Detect valid weak_ptr types.
            // @see https://www.fluentcpp.com/2017/06/02/write-template-metaprogramming-expressively/
            template<class T, class = void_t<>>
            struct is_weak_ptr : std::false_type
            {};

            template<class T>
            struct is_weak_ptr< T, void_t<
                decltype(std::declval<T>().expired()),
                decltype(std::declval<T>().lock()),
                decltype(std::declval<T>().reset()) > >
                : std::true_type
            {};

            template<class T>
            constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

            // Detect reference wrappers.
            // @see https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper
            template<class T, class = void_t<> >
            struct is_reference_wrapper : std::false_type
            {};

            template<class T>
            struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type
            {};

            template<class T>
            constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

            // Detect nullptr types.
            // since C++14
            // @see: https://en.cppreference.com/w/cpp/types/is_null_pointer
            template<class T>
            struct is_null_pointer : std::is_same<std::nullptr_t, typename std::remove_cv<T>::type>
            {};

            // since C++17
            template<class T>
            constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

            // Detect member function traits.
            // @see https://stackoverflow.com/questions/28105077/how-can-i-get-the-class-of-a-member-function-pointer
            template<class>
            struct member_function_traits;

            template<class R, class T, class... Args>
            struct member_function_traits<R(T::*)(Args...)>
            {
                using return_type = R;
                using class_type = T;
                using reference_type = T&;
            };

            template<class R, class T, class... Args>
            struct member_function_traits<R(T::*)(Args...) const>
            {
                using return_type = R;
                using class_type = T;
                using reference_type = const T&;
            };

            // Detect template specialization.
            // @see https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
            template<class Type, template<class...> class Template>
            struct is_specialization : std::false_type
            {};

            template<template<class...> class Template, typename... Types>
            struct is_specialization<Template<Types...>, Template> : std::true_type
            {};

            template<class Type, template<class...> class Template>
            constexpr bool is_specialization_v = is_specialization<Type, Template>::value;

        } // namespace traits

        /**
         * Invoke a pointer to a member function of an object.
         */
        struct invoke_pmf_object
        {
            template<class F, class T, class... Args>
            static constexpr auto call(F pmf, T&& t, Args&&... args)
                noexcept(noexcept((std::forward<T>(t).*pmf)(std::forward<Args>(args)...)))
                -> decltype((std::forward<T>(t).*pmf)(std::forward<Args>(args)...))
            {
                return (std::forward<T>(t).*pmf)(std::forward<Args>(args)...);
            }
        };

        /**
         * Invoke a pointer to a member function on a std::reference_wrapper
         */
        struct invoke_pmf_refwrap
        {
            template<class F, class T, class... Args>
            static constexpr auto call(F pmf, T&& t, Args&&... args)
                noexcept(noexcept((std::forward<T>(t).get().*pmf)(std::forward<Args>(args)...)))
                -> decltype((std::forward<T>(t).get().*pmf)(std::forward<Args>(args)...))
            {
                return (std::forward<T>(t).get().*pmf)(std::forward<Args>(args)...);
            }
        };

        /**
         * Invoke a pointer to member function on a (smart) pointer.
         */
        struct invoke_pmf_pointer
        {
            template<class F, class T, class... Args>
            static constexpr auto call(F pmf, T&& t, Args&&... args)
                noexcept(noexcept(((*std::forward<T>(t)).*pmf)(std::forward<Args>(args)...)))
                -> decltype(((*std::forward<T>(t)).*pmf)(std::forward<Args>(args)...))
            {
                return ((*std::forward<T>(t)).*pmf)(std::forward<Args>(args)...);
            }
        };

        /**
         * Invoke a pointer to a member data on an object.
         */
        struct invoke_pmd_object
        {
            template<class D, class T>
            static constexpr auto call(D pmd, T&& t)
                noexcept(noexcept(std::forward<T>(t).*pmd))
                -> decltype(std::forward<T>(t).*pmd)
            {
                return std::forward<T>(t).*pmd;
            }
        };

        /**
         * Invoke a pointer to member data on a reference wrapper.
         */
        struct invoke_pmd_refwrap
        {
            template<class D, class T>
            static constexpr auto call(D pmd, T&& t)
                noexcept(noexcept(std::forward<T>(t).get().*pmd))
                -> decltype(std::forward<T>(t).get().*pmd)
            {
                return std::forward<T>(t).get().*pmd;
            }
        };

        /**
         * Invoke a pointer to member data on a (smart) pointer.
         */
        struct invoke_pmd_pointer
        {
            template<class D, class T>
            static constexpr auto call(D pmd, T&& t)
                noexcept(noexcept((*std::forward<T>(t)).*pmd))
                -> decltype((*std::forward<T>(t)).*pmd)
            {
                return (*std::forward<T>(t)).*pmd;
            }
        };

        /**
         * Invoke a function object.
         */
        struct invoke_functor
        {
            template<class F, class... Args>
            static constexpr auto call(F&& f, Args&&... args)
                noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
                -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
            {
                return std::forward<F>(f)(std::forward<Args>(args)...);
            }
        };

        // Primary template
        template<class F, class T, class Fd = traits::remove_cvref_t<F>,
            bool is_pmf = traits::is_member_function_pointer_v<Fd>,
            bool is_pmd = traits::is_member_object_pointer_v<Fd>>
            struct invoker_helper;

        // Specialization for pointer to member function.
        template<class F, class T, class Fd>
        struct invoker_helper<F, T, Fd, true, false>
            : traits::conditional_t< traits::is_base_of_v< typename traits::member_function_traits<Fd>::class_type, traits::remove_reference_t<T>>,
            invoke_pmf_object, traits::conditional_t< traits::is_specialization_v< traits::remove_cvref_t<T>, std::reference_wrapper>,
            invoke_pmf_refwrap, invoke_pmf_pointer>>
        {};

        // Specialization for pointer to member data.
        template<class F, class T, class Fd>
        struct invoker_helper<F, T, Fd, false, true>
            : traits::conditional_t< traits::is_base_of_v< typename traits::member_function_traits<Fd>::class_type, traits::remove_reference_t<T>>,
            invoke_pmd_object, traits::conditional_t< traits::is_specialization_v< traits::remove_cvref_t<T>, std::reference_wrapper >,
            invoke_pmd_refwrap, invoke_pmd_pointer>>
        {};

        // Specialization for function objects.
        template<class F, class T, class Fd>
        struct invoker_helper<F, T, Fd, false, false> : invoke_functor
        {};

        // Primary template
        template<class F, class... Args>
        struct invoker;

        // Specialization for 0 arguments.
        template<class F>
        struct invoker<F> : invoke_functor
        {};

        // Specialization for 1 or more arguments.
        template<class F, class T, class... Args>
        struct invoker<F, T, Args...> : invoker_helper<F, T>
        {};

        // since C++17
        // @see https://en.cppreference.com/w/cpp/utility/functional/invoke
        template<class F, class... Args>
        constexpr auto invoke(F&& f, Args&&... args)
            noexcept(noexcept(invoker<F, Args...>::call(std::forward<F>(f), std::forward<Args>(args)...)))
            -> decltype(invoker<F, Args...>::call(std::forward<F>(f), std::forward<Args>(args)...))
        {
            return invoker<F, Args...>::call(std::forward<F>(f), std::forward<Args>(args)...);
        }

        namespace traits
        {
            // Invoke traits when Callable isn't callable with Args
            template<class Void, class... Args>
            struct invoke_traits
            {
                using is_invocable = std::false_type;
                using is_nothrow_invocable = std::false_type;
                template<class R>
                using is_invocable_r = std::false_type;
                template<class R>
                using is_nothrow_invocable_r = std::false_type;
            };

            // Invoke traits when Callable is callable with Args
            template<class... Args>
            struct invoke_traits<void_t<decltype(sig::detail::invoke(std::declval<Args>()...))>, Args...>
            {
                using type = decltype(sig::detail::invoke(std::declval<Args>()...));
                using is_invocable = std::true_type;
                using is_nothrow_invocable = bool_constant<noexcept(sig::detail::invoke(std::declval<Args>()...))>;
                template<class R>
                using is_invocable_r = bool_constant<disjunction_v<std::is_void<R>, std::is_convertible<type, R>>>;
                template<class R>
                using is_nothrow_invocable_r = bool_constant<conjunction_v<is_nothrow_invocable, disjunction<std::is_void<R>, is_nothrow_convertible<type, R>>>>;
            };

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template<class F, class... Args>
            struct is_invocable : invoke_traits<void, F, Args...>::is_invocable
            {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template<class F, class... Args>
            constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template<class F, class... Args>
            struct is_nothrow_invocable : invoke_traits<void, F, Args...>::is_nothrow_invocable
            {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template<class F, class... Args>
            constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<F, Args...>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template <class R, class F, class... Args>
            struct is_invocable_r : invoke_traits<R, F, Args...>::is_invocable_r
            {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template <class R, class F, class... Args>
            constexpr bool is_invocable_r_v = is_invocable_r<R, F, Args...>::value;

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template <class R, class F, class... Args>
            struct is_nothrow_invocable_r : invoke_traits<void, F, Args...>::template is_nothrow_invocable_r<R>
            {};

            // since C++17
            // @see https://en.cppreference.com/w/cpp/types/is_invocable
            template<class R, class F, class... Args>
            constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, Args...>::value;

        } // namespace traits

        template<class F, class... Args>
        struct invoke_result : traits::invoke_traits<void, F, Args...>
        {};

        template<class F, class... Args>
        using invoke_result_t = typename traits::invoke_traits<void, F, Args...>::type;

    } // namespace detail

    /*
     * A slot object holds state information, and a callable to to be called
     * whenever the function call operator is called.
     */
    template <class Func, class Ptr = std::nullptr_t>
    class slot
    {
    public:

        using func_type = detail::traits::decay_t<Func>;
        using ptr_type = detail::traits::decay_t<Ptr>;

        constexpr slot(Func&& f)
            : func{ std::forward<Func>(f) }
            , ptr{ nullptr }
        {}

        constexpr slot(Func&& f, Ptr&& p)
            : func{ std::forward<Func>(f) }
            , ptr{ std::forward<Ptr>(p) }
        {}

        template <class... Args>
        constexpr decltype(auto) operator()(Args&&... args)
            noexcept(
                noexcept(detail::traits::conditional_t<
                    detail::traits::is_null_pointer_v<detail::traits::remove_reference_t<Ptr>>,
                    detail::traits::is_nothrow_invocable<Func, Args...>,
                    detail::traits::is_nothrow_invocable<Func, Ptr, Args...>>::value))
        {
            return do_invoke(func, ptr, std::forward<Args>(args)...);
        }

    private:
        template<class F, class P, class... Args>
        constexpr detail::traits::enable_if_t<detail::traits::is_null_pointer_v<detail::traits::remove_reference_t<P>>, detail::invoke_result_t<F, Args...>>
            do_invoke(F&& f, P&&, Args&&... args) noexcept(detail::traits::is_nothrow_invocable_v<F, Args...>)
        {
            return detail::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }

        template<class F, class P, class... Args>
        constexpr detail::traits::enable_if_t<!detail::traits::is_null_pointer_v<detail::traits::remove_reference_t<P>>, detail::invoke_result_t<F, P, Args...>>
            do_invoke(F&& f, P&& p, Args&&... args) noexcept(detail::traits::is_nothrow_invocable_v<F, P, Args...>)
        {
            return detail::invoke(std::forward<F>(f), std::forward<P>(p), std::forward<Args>(args)...);
        }

        func_type func;
        ptr_type ptr;
    };

    template <class Func>
    slot<Func, std::nullptr_t> make_slot(Func&& f)
    {
        return slot<Func, std::nullptr_t>(std::forward<Func>(f));
    }

    template <class Func, class Ptr>
    slot<Func, Ptr> make_slot(Func&& f, Ptr&& p)
    {
        return slot<Func, Ptr>(std::forward<Func>(f), std::forward<Ptr>(p));
    }

} // namespace sig