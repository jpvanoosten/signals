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

#include <functional>
#include <memory>
#include <type_traits>

namespace sig
{
    namespace detail
    {
        namespace traits
        {
            // Since C++14
            template<class T>
            using decay_t = typename std::decay<T>::type;

            // Since C++14
            template< bool B, class T = void >
            using enable_if_t = typename std::enable_if<B, T>::type;

            // Detect reference wrapper
            // @see https://stackoverflow.com/questions/40430692/how-to-detect-stdreference-wrapper-in-c-at-compile-time
            template <class T>
            struct is_reference_wrapper : std::false_type {};
            template <class U>
            struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

        } // namespace traits

        // Invokes a function object.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<class Func>
        struct invoke_helper
        {
            // Call a function object.
            template<class F, class... Args>
            static auto call(F&& f, Args&&... args) -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
            {
                return std::forward<F>(f)(std::forward<Args>(args)...);
            }
        };

        // Invoke a pointer to member function or pointer to member data.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<class R, class Base>
        struct invoke_helper<R(Base::*)>
        {
            // Get a reference type.
            template<class T, class Td = traits::decay_t<T>,
            class = traits::enable_if_t<std::is_base_of<Base, Td>::value>>
            static auto get(T&& t) -> T&&
            {
                return t;
            }

            // Get a std::reference_wrapper
            template<class T, class Td = traits::decay_t<T>,
            class = traits::enable_if_t<traits::is_reference_wrapper<Td>::value>>
            static auto get(T&& t) -> decltype(t.get())
            {
                return t.get();
            }

            // Get a pointer or pointer-like object (like smart_ptr, or unique_ptr)
            template<class T, class Td = traits::decay_t<T>,
            class = traits::enable_if_t<!std::is_base_of<Base, Td>::value>,
            class = traits::enable_if_t<!traits::is_reference_wrapper<Td>::value>>
            static auto get(T&& t) -> decltype(*std::forward<T>(t))
            {
                return *std::forward<T>(t);
            }

            // Call a pointer to a member function.
            template<class T, class... Args, class MT1,
            class = traits::enable_if_t<std::is_function<MT1>::value>>
            static auto call(MT1 Base::* pmf, T&& t, Args&&... args) 
                -> decltype((get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...))
            {
                return (get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...);
            }

            // Call a pointer to member data.
            template<class T>
            static auto call(R(Base::* pmd), T&& t) 
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
            virtual R operator()(Args&&... args) = 0;
            virtual slot_impl* clone() const = 0;
            virtual ~slot_impl() = default;
        };

        // Slot implementation for callable function objects (Functors)
        template<typename R, typename Func, typename... Args>
        class slot_func : public slot_impl<R, Args...>
        {
        public:
            slot_func(const slot_func&) = default;  // Copy constructor.

            slot_func(Func&& func)
                : m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl* clone() const override
            {
                return new slot_func(*this);
            }

            virtual R operator()(Args&&... args) override
            {
                return invoke_helper<Func>::call(m_Func, std::forward<Args>(args)...);
            }

        private:
            traits::decay_t<Func> m_Func;
        };

        // Slot implementation for pointer to member function and
        // pointer to member data.
        template<typename R, typename Func, typename Ptr, typename... Args>
        class slot_pmf : public slot_impl<R, Args...>
        {
        public:
            slot_pmf(const slot_pmf&) = default;

            slot_pmf(Func&& func, Ptr&& ptr)
                : m_Ptr{ std::forward<Ptr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl* clone() const override
            {
                return new slot_pmf(*this);
            }

            virtual R operator()(Args&&... args) override
            {
                return invoke_helper<Func>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
            }

        private:
            traits::decay_t<Ptr> m_Ptr;
            traits::decay_t<Func> m_Func;
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

        // Slot that takes a pointer to member function.
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

        R operator()(Args&&... args)                // Invoke the signal.
        {
            return (*m_pImpl)(std::forward<Args>(args)...);
        }

    private:
        std::unique_ptr<impl> m_pImpl;              // Pointer to implementation
    };


} // namespace sig