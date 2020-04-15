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
        }

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

            R operator()(Args&&... args)
            {
                return m_Func(std::forward<Args>(args)...);
            }

        private:
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
        : m_pImpl{new detail::slot_func<R, Func, Args...>(std::forward<Func>(func))}
        {}

        // Copy constructor.
        slot(const slot& copy)
        : m_pImpl{copy->m_pImpl->clone()}
        {}

        // Explicit parameterized constructor.
        explicit slot(std::unique_ptr<impl> pImpl)
        : m_pImpl{pImpl}
        {}

        // Move constructor.
        slot(slot&& other)
        : m_pImpl{std::move(other.m_pImpl)}
        {}

        // Assignment operator.
        slot& operator=(const slot& other)
        {
            if ( &other != this )
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