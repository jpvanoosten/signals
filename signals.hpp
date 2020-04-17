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

#include <exception>    // for std::exception
#include <functional>   // for std::reference_wrapper
#include <memory>       // for std::unique_ptr
#include <type_traits>  // for std::decay, and std::enable_if
#include <utility>      // for std::declval.

namespace sig
{
    // An exception of type not_comparable_exception is thrown
    // if one tries to compare non comparable types (like lambdas).
    class not_comparable_exception : public std::exception
    {};

    namespace detail
    {
        namespace traits
        {
            // Since C++14
            template<typename T>
            using decay_t = typename std::decay<T>::type;

            // Since C++14
            template< bool B, typename T = void >
            using enable_if_t = typename std::enable_if<B, T>::type;

            // Detect reference wrapper
            // @see https://stackoverflow.com/questions/40430692/how-to-detect-stdreference-wrapper-in-c-at-compile-time
            template <typename T>
            struct is_reference_wrapper : std::false_type {};
            template <typename U>
            struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

            // Test for equality comparable
            // @see C++ Templates: The Complete Guide (David Vandevoorde, et. al., 2018)
            template<typename T>
            struct is_equality_comparable
            {
            private:
                // Test convertibility of == and !(==) to bool:
                static void* conv(bool);

                template<typename U>
                static std::true_type test(
                    decltype(conv(std::declval<const U&>() == std::declval<const U&>())),
                    decltype(conv(!(std::declval<const U&>() == std::declval<const U&>())))
                );

                // Fallback
                template<typename U>
                static std::false_type test(...);

            public:
                static constexpr bool value = decltype(test<T>(nullptr, nullptr))::value;
            };

        } // namespace traits

        // Use is_equality_compareable to try to perform the equality check
        // (if it is valid for the given type).
        template<typename T, bool = traits::is_equality_comparable<T>::value>
        struct try_equals
        {
            static bool equals(const T& t1, const T& t2)
            {
                return t1 == t2;
            }
        };

        // Partial specialization if type is not equality comparable.
        // In this case, instead of returning false, throw an exception 
        // to indicate that the type is not equality comparable.
        template<typename T>
        struct try_equals<T, false>
        {
            static bool equals(const T&, const T&)
            {
                throw sig::not_comparable_exception();
            }
        };

        // Primary template
        // Invokes a function object.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<typename>
        struct invoke_helper
        {
            // Call a function object.
            template<typename Func, typename... Args>
            static auto call(Func&& f, Args&&... args) -> decltype(std::forward<Func>(f)(std::forward<Args>(args)...))
            {
                return std::forward<Func>(f)(std::forward<Args>(args)...);
            }
        };

        // Invoke a pointer to member function or pointer to member data.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<typename Type, typename Base>
        struct invoke_helper<Type Base::*>
        {
            // Get a reference type.
            template<typename T, typename Td = traits::decay_t<T>,
            typename = traits::enable_if_t<std::is_base_of<Base, Td>::value>>
            static auto get(T&& t) -> T&&
            {
                return t;
            }

            // Get a std::reference_wrapper
            template<typename T, typename Td = traits::decay_t<T>,
            typename = traits::enable_if_t<traits::is_reference_wrapper<Td>::value>>
            static auto get(T&& t) -> decltype(t.get())
            {
                return t.get();
            }

            // Get a pointer or pointer-like object (like smart_ptr, or unique_ptr)
            template<typename T, typename Td = traits::decay_t<T>,
            typename = traits::enable_if_t<!std::is_base_of<Base, Td>::value>,
            typename = traits::enable_if_t<!traits::is_reference_wrapper<Td>::value>>
            static auto get(T&& t) -> decltype(*std::forward<T>(t))
            {
                return *std::forward<T>(t);
            }

            // Call a pointer to a member function.
            template<typename T, typename... Args, typename Type1,
            typename = traits::enable_if_t<std::is_function<Type1>::value>>
            static auto call(Type1 Base::* pmf, T&& t, Args&&... args)
                -> decltype((get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...))
            {
                return (get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...);
            }

            // Call a pointer to member data.
            template<typename T>
            static auto call(Type Base::* pmd, T&& t)
                -> decltype(get(std::forward<T>(t)).*pmd)
            {
                return get(std::forward<T>(t)).*pmd;
            }
        };

        // Base class for slot implementations.
        template<typename R, typename... Args>
        class slot_impl
        {
        public:
            virtual ~slot_impl() = default;
            virtual slot_impl* clone() const = 0;
            virtual bool equals(const slot_impl* s) const = 0;
            virtual R operator()(Args&&... args) = 0;
        };

        // Slot implementation for callable function objects (Functors)
        template<typename R, typename Func, typename... Args>
        class slot_func : public slot_impl<R, Args...>
        {
        public:
            using fuction_type = traits::decay_t<Func>;
            
            slot_func(const slot_func&) = default;  // Copy constructor.

            slot_func(Func&& func)
                : m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_func(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto sfunc = dynamic_cast<const slot_func*>(s))
                {
                    return try_equals<fuction_type>::equals(m_Func, sfunc->m_Func);
                }

                return false;
            }

            virtual R operator()(Args&&... args) override
            {
                return invoke_helper<fuction_type>::call(m_Func, std::forward<Args>(args)...);
            }

        private:
            fuction_type m_Func;
        };

        // Slot implementation for pointer to member function and
        // pointer to member data.
        template<typename R, typename Func, typename Ptr, typename... Args>
        class slot_pmf : public slot_impl<R, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<Ptr>;

            slot_pmf(const slot_pmf&) = default;

            slot_pmf(Func&& func, Ptr&& ptr)
                : m_Ptr{ std::forward<Ptr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_pmf(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto spmf = dynamic_cast<const slot_pmf*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmf->m_Ptr) &&
                           try_equals<function_type>::equals(m_Func, spmf->m_Func);
                }

                return false;
            }

            virtual R operator()(Args&&... args) override
            {
                return invoke_helper<function_type>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

    } // namespace detail

    // Primary template
    template<typename Func>
    class slot;

    // Specialization for function objects.
    template<typename R, typename... Args>
    class slot<R(Args...)>
    {
    public:
        using impl = detail::slot_impl<R, Args...>;

        // Default constructor.
        constexpr slot() noexcept = default;

        // Slot that takes a function object.
        template<typename Func>
        slot(Func&& func)
            : m_pImpl{ new detail::slot_func<R, Func, Args...>(std::forward<Func>(func)) }
        {}

        // Slot that takes a pointer to member function or pointer to member data.
        template<typename Func, typename Ptr>
        slot(Func&& func, Ptr&& ptr)
            : m_pImpl{ new detail::slot_pmf<R, Func, Ptr, Args...>(std::forward<Func>(func), std::forward<Ptr>(ptr)) }
        {}

        // Copy constructor.
        slot(const slot& copy)
            : m_pImpl{ copy->m_pImpl->clone() }
        {}

        // Explicit parameterized constructor.
        explicit slot(std::unique_ptr<impl> pImpl)
            : m_pImpl{ pImpl }
        {}

        // Move constructor.
        slot(slot&& other)
            : m_pImpl{ std::move(other.m_pImpl) }
        {}

        // Assignment operator.
        slot& operator=(const slot& other)
        {
            if (&other != this)
            {
                m_pImpl.swap(other.m_pImpl->clone());
            }
            return *this;
        }

        // Move assignment operator.
        slot& operator=(slot&& other)
        {
            m_pImpl = std::move(other.m_pImpl);
            return *this;
        }

        // Explicit conversion to bool.
        explicit operator bool() const
        {
            return m_pImpl != nullptr;
        }

        // Equality operator
        friend bool operator==(const slot& s1, const slot& s2)
        {
            if (!s1 || !s2)
            {
                return !s1 && !s2;
            }

            return s1.m_pImpl->equals(s2.m_pImpl.get());
        }

        // Inequality operator
        friend bool operator!=(const slot& s1, const slot& s2)
        {
            return !(s1 == s2);
        }

        // Invoke the signal.
        R operator()(Args&&... args)
        {
            return (*m_pImpl)(std::forward<Args>(args)...);
        }

    private:
        std::unique_ptr<impl> m_pImpl;              // Pointer to implementation
    };


} // namespace sig