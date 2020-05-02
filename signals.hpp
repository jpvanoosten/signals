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

#include "optional.hpp" // for opt::optional
#include <atomic>       // for std::atomic_bool
#include <cstddef>      // for std::size_t and std::nullptr_t
#include <exception>    // for std::exception
#include <functional>   // for std::reference_wrapper
#include <memory>       // for std::unique_ptr
#include <mutex>        // for std::mutex, and std::lock_guard
#include <tuple>        // for std::tuple, and std::make_tuple
#include <type_traits>  // for std::decay, and std::enable_if
#include <utility>      // for std::declval.
#include <vector>       // for std::vector

namespace sig
{
    // An exception of type not_comparable_exception is thrown
    // if one tries to compare non comparable types (like lambdas).
    class not_comparable_exception : public std::exception
    {};

    // Pointers that can be converted to a weak pointer concept for 
    // tracking purposes must implement the to_weak() function in order
    // to make use of Argument-dependent lookup (ADL) and to convert
    // it to a type whose lifetime can be tracked by the slot.
    template<typename T>
    std::weak_ptr<T> to_weak(std::weak_ptr<T> w)
    {
        return w;
    }

    template<typename T>
    std::weak_ptr<T> to_weak(std::shared_ptr<T> s)
    {
        return s;
    }

    namespace detail
    {
        namespace traits
        {
            // Since C++17
            // @see https://en.cppreference.com/w/cpp/types/void_t
            template<typename... Ts>
            struct make_void
            {
                typedef void type;
            };

            template<typename... Ts>
            using void_t = typename make_void<Ts...>::type;

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

            // Detect if type supports weak_ptr semantics.
            template<typename T, typename = void>
            struct is_weak_ptr : std::false_type
            {};

            template<typename T>
            struct is_weak_ptr<T, void_t<decltype(std::declval<T>().expired()),
                decltype(std::declval<T>().lock()),
                decltype(std::declval<T>().reset())>>
                : std::true_type
            {};

            // Test to see if a type can be converted to a weak_ptr type.
            template <typename T, typename = void>
            struct is_weak_ptr_convertable : std::false_type
            {};

            template <typename T>
            struct is_weak_ptr_convertable<T, void_t<decltype(to_weak(std::declval<T>()))>>
                : is_weak_ptr<decltype(to_weak(std::declval<T>()))>
            {};

            // Primary template for extracting function traits.
            template<typename Func>
            struct function_traits;

            template<typename R, typename... Args>
            struct function_traits<R(Args...)>
            {
                using result_type = R;
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
        // In this case, throw an exception to indicate that the type is not 
        // equality comparable.
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

        /**
         * A copy-on-write template class to avoid unnecessary copies of
         * data unless the data will be modified. This greatly improves
         * the performance of the signal class in a multi-threaded environment
         * since copies only require a shared pointer copy.
         * The copy-on-write pointer has similar semantics to shared pointers.
         *
         * @see API Design for C++, Martin Reddy (Elsevier, 2011).
         */
        template<typename T>
        class cow_ptr
        {
        public:
            using value_type = T;
            using pointer_type = std::shared_ptr<T>;

            constexpr cow_ptr() noexcept = default;
            ~cow_ptr() = default;

            constexpr explicit cow_ptr(T* other) noexcept
                : m_Ptr(other)
            {}

            constexpr cow_ptr(const cow_ptr& other) noexcept
                : m_Ptr(other.m_Ptr)
            {}

            constexpr cow_ptr(cow_ptr&& other) noexcept
                : m_Ptr(std::move(other.m_Ptr))
            {}

            // Copy assignment operator.
            cow_ptr& operator=(const cow_ptr& other) noexcept
            {
                if (this != &other)
                {
                    *this = cow_ptr(other);
                }

                return *this;
            }

            // Move assignment operator.
            cow_ptr& operator=(cow_ptr&& other) noexcept
            {
                m_Ptr = std::move(other.m_Ptr);
                return *this;
            }

            // Assign from other value.
            cow_ptr& operator=(T* other) noexcept
            {
                m_Ptr = pointer_type(other);
                return *this;
            }

            // Non-const dereference operator.
            // Will create a copy of the underlying object.
            T& operator*()
            {
                detach();
                return *m_Ptr;
            }

            // Const dereference operator.
            // No copy is made of the underlying object
            const T& operator*() const noexcept
            {
                return *m_Ptr;
            }

            // Non-const pointer dereference operator.
            // Will create a copy of the underlying object.
            T* operator->()
            {
                detach();
                return m_Ptr.get();
            }

            // Const pointer dereference operator.
            // No copy is made of the underlying object.
            const T* operator->() const
            {
                return m_Ptr.get();
            }

            // Non-const implicit conversion of underlying type.
            // A copy will be created of the underlying object.
            operator T* ()
            {
                detach();
                return m_Ptr.get();
            }

            // Const implicit conversion of underlying type.
            // No copy is made of the underlying object.
            operator const T* () const
            {
                return m_Ptr.get();
            }

            T* data()
            {
                detach();
                return m_Ptr.get();
            }

            const T* data() const
            {
                return m_Ptr.get();
            }

            // Get read-only reference to internal value.
            // No copy is made of the underlying object.
            const T& read() const
            {
                return *m_Ptr;
            }

            // Get writable reference to internal value.
            // A copy will be created of the underlying type.
            T& write()
            {
                detach();
                return *m_Ptr;
            }

            template<typename U>
            bool operator==(const cow_ptr<U>& rhs) const noexcept
            {
                return m_Ptr == rhs.m_Ptr;
            }

            template<typename U>
            bool operator!=(const cow_ptr<U>& rhs) const noexcept
            {
                return m_Ptr != rhs.m_Ptr;
            }

            // Explicit bool conversion to check for a valid
            // internal value.
            explicit operator bool() const noexcept
            {
                return bool(m_Ptr);
            }

        private:
            void detach()
            {
                if (m_Ptr && m_Ptr.use_count() > 1)
                {
                    // Detach from the shared pointer
                    // creating a new instance of the stored object.
                    *this = cow_ptr(new T(*m_Ptr));
                }
            }

            pointer_type m_Ptr;
        };

        // Utility function to create a cow_ptr.
        template<typename T, typename... Args>
        cow_ptr<T> make_cow(Args&&... args)
        {
            return cow_ptr<T>(new T(std::forward<Args>(args)...));
        }

        template<typename T, typename U, typename... Args>
        cow_ptr<T> make_cow(std::initializer_list<U> il, Args&&... args)
        {
            return cow_ptr<T>(new T(il, std::forward<Args>(args)...));
        }

        /**
         * Slot state is used as both a non-template base class for slots
         * as well as storing connection information about the slot.
         */
        class slot_state
        {
        public:
            constexpr slot_state() noexcept
                : m_Index(0)
                , m_Connected(true)
                , m_Blocked(false)
            {}

            // Atomic variables are not CopyConstructible.
            // @see https://en.cppreference.com/w/cpp/atomic/atomic/atomic
            slot_state(const slot_state& s) noexcept
                : m_Index(s.m_Index)
                , m_Connected(s.m_Connected.load())
                , m_Blocked(s.m_Blocked.load())
            {}

            slot_state(slot_state&& s) noexcept
                : m_Index(s.m_Index)
                , m_Connected(s.m_Connected.load())
                , m_Blocked(s.m_Blocked.load())
            {}

            virtual ~slot_state() = default;

            slot_state& operator=(const slot_state& s) noexcept
            {
                m_Index = s.m_Index;
                m_Connected = s.m_Connected.load();
                m_Blocked = s.m_Blocked.load();

                return *this;
            }

            slot_state& operator=(slot_state&& s) noexcept
            {
                m_Index = s.m_Index;
                m_Connected.store(s.m_Connected.load());
                m_Blocked.store(s.m_Blocked.load());

                return *this;
            }

            virtual bool connected() const noexcept
            {
                return m_Connected;
            }

            bool disconnect() noexcept
            {
                bool ret = m_Connected.exchange(false);
                if (ret)
                {
                    do_disconnect();
                }

                return ret;
            }

            bool blocked() const noexcept
            {
                return m_Blocked;
            }

            void block() noexcept
            {
                m_Blocked = true;
            }

            void unblock() noexcept
            {
                m_Blocked = false;
            }

            std::size_t& index()
            {
                return m_Index;
            }

        protected:
            virtual void do_disconnect()
            {}

        private:
            std::size_t m_Index;
            std::atomic_bool m_Connected;
            std::atomic_bool m_Blocked;
        };

        // Base class for slot implementations.
        template<typename R, typename... Args>
        class slot_impl : public slot_state
        {
        public:
            virtual ~slot_impl() = default;
            virtual slot_impl* clone() const = 0;
            virtual bool equals(const slot_impl* s) const = 0;
            virtual opt::optional<R> operator()(Args&&... args) = 0;
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

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                return invoke_helper<fuction_type>::call(m_Func, std::forward<Args>(args)...);
            }

        private:
            fuction_type m_Func;
        };

        // Specialization for void return types.
        template<typename Func, typename... Args>
        class slot_func<void, Func, Args...> : public slot_impl<void, Args...>
        {
        public:
            using fuction_type = traits::decay_t<Func>;

            slot_func(const slot_func&) = default;  // Copy constructor.

            slot_func(Func&& func)
                : m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_func(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto sfunc = dynamic_cast<const slot_func*>(s))
                {
                    return try_equals<fuction_type>::equals(m_Func, sfunc->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                return (invoke_helper<fuction_type>::call(m_Func, std::forward<Args>(args)...), opt::nullopt);
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

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                return invoke_helper<function_type>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function and
        // pointer to member data.
        // Specialized on void return value.
        template<typename Func, typename Ptr, typename... Args>
        class slot_pmf<void, Func, Ptr, Args...> : public slot_impl<void, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<Ptr>;

            slot_pmf(const slot_pmf&) = default;

            slot_pmf(Func&& func, Ptr&& ptr)
                : m_Ptr{ std::forward<Ptr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_pmf(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto spmf = dynamic_cast<const slot_pmf*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmf->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmf->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                invoke_helper<function_type>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function that automatically 
        // tracks the lifetime of a supplied object through a weak pointer in 
        // order to disconnect the slot on object destruction.
        template<typename R, typename Func, typename WeakPtr, typename... Args>
        class slot_pmf_tracked : public slot_impl<R, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<WeakPtr>;

            slot_pmf_tracked(const slot_pmf_tracked&) = default;

            slot_pmf_tracked(Func&& func, WeakPtr&& ptr)
                : m_Ptr{ std::forward<WeakPtr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_pmf_tracked(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto spmft = dynamic_cast<const slot_pmf_tracked*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmft->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmft->m_Func);
                }

                return false;
            }

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                auto sp = m_Ptr.lock();
                if (!sp)
                {
                    this->disconnect();
                    return {};
                }

                if (this->connected())
                {
                    return invoke_helper<function_type>::call(m_Func, sp, std::forward<Args>(args)...);
                }

                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function that automatically 
        // tracks the lifetime of a supplied object through a weak pointer in 
        // order to disconnect the slot on object destruction.
        // Partial specialization for void return types.
        template<typename Func, typename WeakPtr, typename... Args>
        class slot_pmf_tracked<void, Func, WeakPtr, Args...> : public slot_impl<void, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<WeakPtr>;

            slot_pmf_tracked(const slot_pmf_tracked&) = default;

            slot_pmf_tracked(Func&& func, WeakPtr&& ptr)
                : m_Ptr{ std::forward<WeakPtr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_pmf_tracked(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto spmft = dynamic_cast<const slot_pmf_tracked*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmft->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmft->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                auto sp = m_Ptr.lock();
                if (!sp)
                {
                    this->disconnect();
                }

                if (this->connected())
                {
                    invoke_helper<function_type>::call(m_Func, sp, std::forward<Args>(args)...);
                }

                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // since C++14
        // @see https://en.cppreference.com/w/cpp/utility/integer_sequence
        // @see https://gist.github.com/ntessore/dc17769676fb3c6daa1f
        // Integer sequence is used to unpack a tuple. This is required for the 
        // slot iterator.
        template<typename T, T... Ints>
        struct integer_sequence
        {
            using value_type = T;
            static constexpr std::size_t size()
            {
                return sizeof...(Ints);
            }
        };

        template<std::size_t... Ints>
        using index_sequence = integer_sequence<std::size_t, Ints...>;

        template<typename T, std::size_t N, T... Is>
        struct make_integer_sequence : make_integer_sequence<T, N-1, N-1, Is...>
        {};

        // Specialization for integer sequences of 0 length.
        template<typename T, T... Is>
        struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...>
        {};

        template<std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;

        template<typename... T>
        using index_sequence_for = make_index_sequence<sizeof...(T)>;

        // The slot_iterator is a wrapper for the actual container that 
        // contains a list of slots to be invoked. When the slot_iterator
        // is dereferenced, it must invoke the slot that referenced by the 
        // current internal iterator and return the result of invoking the function.
        template<typename T, typename InputIterator, typename... Args>
        class slot_iterator
        {
        public:
            // These are required for iterators:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            using args_type = std::tuple<Args...>;
            using args_sequence = make_index_sequence<sizeof...(Args)>;

            slot_iterator(InputIterator iter, args_type& args)
                : m_Iter(iter)
                , m_Args(args)
            {}

            slot_iterator(const slot_iterator&) = default;
            slot_iterator(slot_iterator&&) = default;

            // Pre-increment operator.
            slot_iterator& operator++()
            {
                ++m_Iter;
                return *this;
            }

            // Post-increment operator
            slot_iterator operator++(int)
            {
                slot_iterator tmp(*this);
                ++m_Iter;
                return tmp;
            }

            bool operator==(const slot_iterator& other) const
            {
                return m_Iter == other.m_Iter;
            }

            bool operator!=(const slot_iterator& other) const
            {
                return m_Iter != other.m_Iter;
            }

            // Invoke the slot referenced by the internal iterator.
            opt::optional<T> operator*()
            {
                return do_invoke(args_sequence());
            }

        private:
            template<std::size_t... Is>
            constexpr opt::optional<T> do_invoke(index_sequence<Is...>)
            {
                // Unpack tuple arguments and invoke slot.
                return (*m_Iter)(std::get<Is>(m_Args)...);
            }

            InputIterator m_Iter;
            args_type& m_Args;
        };

        template<typename InputIterator, typename... Args>
        class slot_iterator<void, InputIterator, Args...>
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = void;
            using difference_type = std::ptrdiff_t;
            using pointer = void;
            using reference = void;

            using args_type = std::tuple<Args...>;
            using args_sequence = make_index_sequence<sizeof...(Args)>;

            slot_iterator(InputIterator iter, args_type& args)
                : m_Iter(iter)
                , m_Args(args)
            {}

            slot_iterator(const slot_iterator&) = default;
            slot_iterator(slot_iterator&&) = default;

            // Pre-increment operator.
            slot_iterator& operator++()
            {
                ++m_Iter;
                return *this;
            }

            // Post-increment operator
            slot_iterator operator++(int)
            {
                slot_iterator tmp(*this);
                ++m_Iter;
                return tmp;
            }

            bool operator==(const slot_iterator& other) const
            {
                return m_Iter == other.m_Iter;
            }

            bool operator!=(const slot_iterator& other) const
            {
                return m_Iter != other.m_Iter;
            }

            opt::optional<void> operator*()
            {
                return do_invoke(args_sequence());
            }

        private:
            template<std::size_t... Is>
            constexpr opt::optional<void> do_invoke(index_sequence<Is...>)
            {
                // Unpack tuple arguments and invoke slot.
                (*m_Iter)(std::get<Is>(m_Args)...);
                return {};
            }

            InputIterator m_Iter;
            args_type& m_Args;
        };
    } // namespace detail

    // Primary slot template
    template<typename Func>
    class slot;

    // Specialization for function objects.
    template<typename R, typename... Args>
    class slot<R(Args...)>
    {
        using impl = detail::slot_impl<R, Args...>;

    public:
        // Default constructor.
        constexpr slot() noexcept = default;

        // Slot that takes a function object.
        template<typename Func>
        slot(Func&& func)
            : m_pImpl{ new detail::slot_func<R, Func, Args...>(std::forward<Func>(func)) }
        {}

        // Slot that takes a pointer to member function or pointer to member data.
        template<typename Func, typename Ptr>
        slot(Func&& func, Ptr&& ptr, detail::traits::enable_if_t<!detail::traits::is_weak_ptr_convertable<Ptr>::value, void*> = nullptr)
            : m_pImpl{ new detail::slot_pmf<R, Func, Ptr, Args...>(std::forward<Func>(func), std::forward<Ptr>(ptr)) }
        {}

        template<typename Func, typename Ptr>
        slot(Func&& func, Ptr&& ptr, detail::traits::enable_if_t<detail::traits::is_weak_ptr_convertable<Ptr>::value, void*> = nullptr)
            : m_pImpl{ new detail::slot_pmf_tracked<R, Func, decltype(to_weak(std::forward<Ptr>(ptr))), Args...>(std::forward<Func>(func), to_weak(std::forward<Ptr>(ptr))) }
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
        slot& operator=(slot&& other) noexcept
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

        bool connected() const noexcept
        {
            return m_pImpl && m_pImpl->connected();
        }

        bool disconnect() noexcept
        {
            return m_pImpl && m_pImpl->disconnect();
        }

        bool blocked() const noexcept
        {
            return m_pImpl && m_pImpl->blocked();
        }

        void block() noexcept
        {
            if ( m_pImpl ) 
                m_pImpl->block();
        }

        void unblock() noexcept
        {
            if ( m_pImpl )
                m_pImpl->unblock();
        }

        // Invoke the slot.
        opt::optional<R> operator()(Args&&... args)
        {
            return (*m_pImpl)(std::forward<Args>(args)...);
        }

    private:
        // Signals need to access the state of the slots.
        template<typename Func, typename Combiner>
        friend class signal;

        std::size_t& index()
        {
            return m_pImpl->index();
        }

        // Query the state of the slot.
        constexpr detail::slot_state& state()
        {
            return *m_pImpl;
        }

        std::unique_ptr<impl> m_pImpl;              // Pointer to implementation
    };

    // Shared pointer to a slot.
    template<typename T>
    using slot_ptr = std::shared_ptr<slot<T>>;

    // Weak pointer to a slot.
    template<typename T>
    using slot_wptr = std::weak_ptr<slot<T>>;

    /**
     * An RAII object that blocks connections until destruction.
     */
    template<typename Func>
    class connection_blocker
    {
    public:
        using slot_type = slot_wptr<Func>;

        connection_blocker() noexcept = default;
        ~connection_blocker() noexcept
        {
            release();
        }

        connection_blocker(const connection_blocker&) noexcept = delete;
        connection_blocker(connection_blocker&& c) noexcept
            : m_Slot{ std::move(c.m_Slot) }
        {}

        connection_blocker& operator=(const connection_blocker&) = delete;
        connection_blocker& operator=(connection_blocker&& c) noexcept
        {
            release();
            m_Slot = std::move(c.m_Slot);
            return *this;
        }

    private:
        template<typename>
        friend class connection;

        explicit connection_blocker(slot_type slot) noexcept
            : m_Slot{ std::move(slot) }
        {
            if (auto s = m_Slot.lock())
            {
                s->block();
            }
        }

        void release() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->unblock();
            }
        }

        slot_type m_Slot;
    };

    /**
     * Connection object allows for managing slot connections.
     */
    template<typename Func>
    class connection
    {
    public:
        using slot_type = slot_wptr<Func>;

        connection() noexcept = default;
        virtual ~connection() = default;

        connection(const connection&) noexcept = default;
        connection(connection&&) noexcept = default;
        connection& operator=(const connection&) noexcept = default;
        connection& operator=(connection&&) noexcept = default;

        bool valid() const noexcept
        {
            return !m_Slot.expired();
        }

        bool connected() const noexcept
        {
            const auto s = m_Slot.lock();
            return s && s->connected();
        }

        bool disconnect() noexcept
        {
            auto s = m_Slot.lock();
            return s && s->disconnect();
        }

        bool blocked() const noexcept
        {
            const auto s = m_Slot.lock();
            return s && s->blocked();
        }

        void block() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->block();
            }
        }

        void unblock() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->unblock();
            }
        }

        connection_blocker<Func> blocker() const noexcept
        {
            return connection_blocker<Func>(m_Slot);
        }

    protected:
        template<typename, typename>
        friend class signal;

        explicit connection(slot_type s) noexcept
            : m_Slot{ std::move(s) }
        {}

        slot_type m_Slot;
    };

    /**
     * An RAII version of connection which disconnects it's slot on destruction.
     */
    template<typename Func>
    class scoped_connection : public connection<Func>
    {
    public:
        using base = connection<Func>;
        using slot_type = slot_wptr<Func>;

        scoped_connection() = default;
        virtual ~scoped_connection() override
        {
            this->disconnect();
        }

        scoped_connection(const base& c) noexcept
            : base(c)
        {}

        scoped_connection(base&& c) noexcept
            : base(std::move(c))
        {}

        scoped_connection(const scoped_connection&) noexcept = delete;

        scoped_connection(scoped_connection&& s) noexcept
            : base(std::move(s.m_Slot))
        {}

        scoped_connection& operator=(const scoped_connection&) noexcept = delete;

        scoped_connection& operator=(scoped_connection&& c) noexcept
        {
            this->disconnect();
            this->m_Slot = std::move(c.m_Slot);
            return *this;
        }

    private:
        explicit scoped_connection(slot_type s) noexcept
            : base(std::move(s))
        {}
    };

    // Default combiner for signals returns an optional value.
    // The combiner just returns the result of the last connected slot.
    // If no slots or functions returns void, a disengaged optional is returned.
    template<typename T>
    class optional_last_value
    {
    public:
        using result_type = opt::optional<T>;

        template<typename InputIterator>
        result_type operator()(InputIterator first, InputIterator last) const
        {
            result_type result;
            while (first != last)
            {
                result_type temp = *first;
                if (temp)   // Skip disengaged results.
                    result = temp;
                ++first;
            }

            return result;
        }
    };

    // Primary template for the signal.
    template<typename Func, typename Combiner = optional_last_value<typename detail::traits::function_traits<Func>::result_type>>
    class signal;

    // Partial specialization taking a callable.
    template<typename R, typename... Args, typename Combiner>
    class signal<R(Args...), Combiner>
    {
    public:
        using slot_type = slot_ptr<R(Args...)>;
        using connection_type = connection<R(Args...)>;
        using list_type = std::vector<slot_type>;
        using list_iterator = typename list_type::iterator;
        using cow_type = detail::cow_ptr<list_type>;
        using mutex_type = std::mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using result_type = typename Combiner::result_type;

        signal()
            : m_Blocked(false)
        {}
        ~signal() = default;

        // Not copyable.
        signal(const signal&) = delete;
        // Not copy assignable.
        signal& operator=(const signal&) = delete;

        // Moveable.
        signal(signal&& other) noexcept
            : m_Blocked(other.m_Blocked.load())
        {
            lock_type lock(other.m_SlotMutex);
            m_Slots = std::move(other.m_Slots);
        }

        // Move assignable.
        signal& operator=(signal&& other) noexcept
        {
            lock_type lock1(m_SlotMutex, std::defer_lock);
            lock_type lock2(other.m_SlotMutex, std::defer_lock);
            std::lock(lock1, lock2);
            
            m_Slots = std::move(other.m_Slots);
            m_Blocked = other.m_Blocked.load();
        }

        // Connect a slot with a callable function object.
        template<typename Func>
        connection_type connect(Func&& f)
        {
            slot_type s = slot_type(new slot<R(Args...)>(std::forward<Func>(f)));
            connection_type c(s);
            add_slot(std::move(s));
            return c;
        }

        // Connect a slot with a pointer to member function
        // or pointer to member data.
        template<typename Func, typename Ptr>
        connection_type connect(Func&& f, Ptr&& p)
        {
            slot_type s = slot_type(new slot<Func>(std::forward<Func>(f), std::forward<Ptr>(p)));
            connection_type c(s);
            add_slot(std::move(s));
            return c;
        }

        // Disconnect any slots that are bound to the function object.
        // Returns the number of slots that were disconnected.
        template<typename Func>
        std::size_t disconnect(Func&& f)
        {
            // Create a temporary slot for comparison.
            auto s = slot<Func>(std::forward<Func>(f));
            return remove_slot(s);
        }

        // Disconnect any slots that are bound to the function object.
        // Returns the number of slots that were disconnected.
        template<typename Func, typename Ptr>
        std::size_t disconnect(Func&& f, Ptr&& ptr)
        {
            // Create a temporary slot for comparison.
            auto s = slot<Func>(std::forward<Func>(f), std::forward<Ptr>(ptr));
            return remove_slot(s);
        }

        result_type operator()(Args... _args)
        {
            if ( m_Blocked ) return {};

            auto args = std::make_tuple(std::forward<Args>(_args)...);
            auto& slots = m_Slots.read();

            using iterator = detail::slot_iterator<R, list_iterator, Args...>;
            return Combiner()(iterator(slots.begin(), args), iterator(slots.end(), args));
        }

    private:

        void add_slot(slot_type&& s)
        {
            lock_type lock(m_SlotMutex);
            
            auto& slots = m_Slots.write();
            s->state().index() = slots.size();
            slots.push_back(std::move(s));
        }

        template<typename Func>
        size_t remove_slot(slot<Func>& slot)
        {
            // Get a modifiable copy of the slot list.
            auto& slots = m_Slots.write();

            std::size_t count = 0;   // The number of slots that were removed.
            std::size_t i = 0;       // Slot index
            while (i < slots.size())
            {
                auto& s = *slots[i];
                if (s == slot)
                {
                    std::swap(slots[i], slots.back());
                    slots[i]->index() = i;
                    slots.pop_back();
                    ++count;
                }
                else
                {
                    ++i;
                }
            }
            return count;
        }

        void clear()
        {
            lock_type lock(m_SlotMutex);
            m_Slots->clear();
        }

        mutex_type m_SlotMutex;
        cow_type m_Slots;
        std::atomic_bool m_Blocked;
    };
} // namespace sig