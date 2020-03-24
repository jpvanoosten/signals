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
 */

#include <atomic>

namespace signals
{

namespace detail
{

/**
 * Represents a list of types.
 */
template <typename...>
struct typelist {};

using data_ptr = const void*;
using call_pptr = data_ptr*;


/**
 * slot_state holds slot data that is independent of the slot type.
 */
class slot_state {
public:
    constexpr slot_state() noexcept
        : m_index(0)
        , m_connected(true)
        , m_blocked(false)
    {}

    virtual ~slot_state() = default;

    virtual bool connected() const noexcept { return m_connected; }

    bool disconnect() noexcept {
        bool ret = m_connected.exchange(false);
        if (ret) {
            do_disconnect();
        }
        return ret;
    }

    bool blocked() const noexcept { return m_blocked.load(); }
    void block()   noexcept { m_blocked.store(true); }
    void unblock() noexcept { m_blocked.store(false); }

protected:
    virtual void do_disconnect() {}
    std::size_t& index() {
        return m_index;
    }


private:
    template <typename, typename...>
    friend class ::signals::signal_base;

    std::size_t m_index;  // index into the array of slot pointers inside the signal
    std::atomic<bool> m_connected;
    std::atomic<bool> m_blocked;
};

/**
 * Interface for cleanable objects, used to cleanup disconnected slots.
 */
struct cleanable {
    virtual ~cleanable() = default;
    virtual void clean(slot_state*) = 0;
};

/**
 * A base class for slot objects. This base type only depends on slot argument
 * types, it will be used as an element in a list of slots, hence the public next member.
 */
template <typename R, typename... Args>
class slot_base : public slot_state {
public:

    using return_type = R;
    using args_type = typelist<Args...>;
    using func_type = std::function<R, Args...>;

    explicit slot_base(cleanable& c) : cleaner(c) {}
    ~slot_base() override = default;

    // method effectively responsible for calling the "slot" function with
    // supplied arguments whenever emission happens.
    virtual R call_slot(Args...) = 0;

    template <typename... U>
    R operator()(U&& ...u) {
        if (slot_state::connected() && !slot_state::blocked()) {
            return call_slot(std::forward<U>(u)...);
        }
    }

    // Retrieve a pointer to the object embedded in the slot
    virtual data_ptr get_object() const noexcept {
        return nullptr;
    }

    // Retrieve a pointer to the callable embedded in the slot
    virtual void get_callable(call_pptr p) const noexcept {
        *p = nullptr;
    }

protected:

    void do_disconnect() final {
        cleaner.clean(this);
    }

private:
    cleanable& cleaner;
};

/*
 * A slot object holds state information, and a callable to to be called
 * whenever the function call operator of its slot_base base class is called.
 */
template <typename R, typename... Args>
class slot : public slot_base<R, Args...> {
public:

    template <typename F>
    constexpr slot(cleanable& c, F&& f)
        : slot_base<R, Args...>(c)
        , func{ std::forward<F>(f) } {}

protected:
    R call_slot(Args ...args) override {
        return func(args...);
    }

    void target(call_pptr p) const noexcept override {
        *p = func.target<func_type>();
    }

private:
    std::function<R, Args...> func;
};



}

}