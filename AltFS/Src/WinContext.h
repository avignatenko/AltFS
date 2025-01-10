#pragma once

#include <function2/function2.hpp>

struct minimal_io_executor
{
    const unsigned int kCustomMessage = WM_USER + 1;

    asio::execution_context* context_;
    DWORD threadId_;

    bool operator==(const minimal_io_executor& other) const noexcept
    {
        return context_ == other.context_ && threadId_ == other.threadId_;
    }

    bool operator!=(const minimal_io_executor& other) const noexcept { return !(*this == other); }

    asio::execution_context& query(asio::execution::context_t) const noexcept { return *context_; }

    static constexpr asio::execution::blocking_t::never_t query(asio::execution::blocking_t) noexcept
    {
        // This executor always has blocking.never semantics.
        return asio::execution::blocking.never;
    }

    template <class F>
    void execute(F f) const
    {
        auto* t = new fu2::unique_function<void()>([f = std::forward<F>(f)]() mutable { std::move(f)(); });
        ::PostThreadMessage(threadId_, kCustomMessage, (WPARAM)t, 0);
    }
};

namespace asio
{
namespace traits
{

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename F>
struct execute_member<minimal_io_executor, F>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef void result_type;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <>
struct equality_comparable<minimal_io_executor>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <>
struct query_member<minimal_io_executor, asio::execution::context_t>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef asio::execution_context& result_type;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Property>
struct query_static_constexpr_member<
    minimal_io_executor, Property,
    typename enable_if<std::is_convertible<Property, asio::execution::blocking_t>::value>::type>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef asio::execution::blocking_t::never_t result_type;
    static constexpr result_type value() noexcept { return result_type(); }
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

}  // namespace traits
}  // namespace asio

class WinContext : public asio::execution_context
{
public:
    WinContext() { threadId_ = ::GetCurrentThreadId(); }

    auto get_executor()
    {
        minimal_io_executor e;
        e.context_ = this;
        e.threadId_ = threadId_;
        return e;
    }

    void process(const MSG& msg)
    {
        if (msg.message == WM_USER + 1)
        {
            auto* t = (fu2::unique_function<void()>*)msg.wParam;
            (*t)();
            delete t;
        }
    }

private:
    DWORD threadId_;
};
