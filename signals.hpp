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

/*
 * A slot object holds state information, and a callable to to be called
 * whenever the function call operator is called.
 */
template <typename Func, typename Ptr = std::nullptr_t>
class slot
{
public:
    constexpr slot(Func&& f)
        : func{ std::forward<Func>(f) }
        , ptr{ nullptr }
    {}

    constexpr slot(Func&& f, Ptr&& p)
        : func{ std::forward<Func>(f) }
        , ptr{ std::forward<Ptr>(p) }
    {}

    template <typename... Args>
    decltype(auto) operator()(Args&&... args)
    {
        return do_invoke(func, ptr, std::forward<Args>(args)...);
    }

private:
    template<typename F, typename P, typename... Args>
    constexpr std::enable_if_t<std::is_null_pointer_v<std::decay_t<P>>, std::invoke_result_t<F, Args...>>
        do_invoke(F&& f, P&&, Args&&... args)
    {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename F, typename P, typename... Args>
    constexpr std::enable_if_t<!std::is_null_pointer_v<std::decay_t<P>>, std::invoke_result_t<F, P, Args...>>
        do_invoke(F&& f, P&& p, Args&&... args)
    {
        return std::invoke(std::forward<F>(f), std::forward<P>(p), std::forward<Args>(args)...);
    }

    std::decay_t<Func> func;
    std::decay_t<Ptr> ptr;
};

template <typename Func>
slot<Func, std::nullptr_t> make_slot(Func&& f)
{
    return slot<Func, std::nullptr_t>(std::forward<Func>(f));
}

template <typename Func, typename Ptr>
slot<Func, Ptr> make_slot(Func&& f, Ptr&& p)
{
    return slot<Func, Ptr>(std::forward<Func>(f), std::forward<Ptr>(p));
}

} // namespace signals