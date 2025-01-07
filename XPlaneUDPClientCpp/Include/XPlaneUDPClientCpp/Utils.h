#pragma once

#include <exception>

#include <continuable/continuable-transforms.hpp>

class WaitOnContextCancelledException : public std::exception
{
public:
    WaitOnContextCancelledException() noexcept = default;

    const char* what() const noexcept override
    {
        return "waitOnContext canceled due to cancellation of the continuation";
    }
};

namespace internal
{

template <typename Data, typename Annotation,
          typename Result = typename cti::detail::transforms::sync_trait<Annotation>::result_t>
Result waitOnContextAndUnpackRelaxed(cti::continuable_base<Data, Annotation>&& continuable, asio::io_context& context)
{
    // Do an immediate unpack if the continuable is ready
    if (continuable.is_ready())
    {
        return std::move(continuable).unpack();
    }

    Result sync_result;

    bool done = false;

    std::move(continuable)
        .next(
            [&sync_result, &done](auto&&... args)
            {
                sync_result = Result::from(std::forward<decltype(args)>(args)...);
                done = true;
            })
        .done();

    // fixme: cpu-intensive loop potentially
    while (!done) context.poll();

    return sync_result;
}

template <typename Data, typename Annotation>
auto waitOnContextAndUnpack(cti::continuable_base<Data, Annotation>&& continuable, asio::io_context& context)
{
    auto sync_result = waitOnContextAndUnpackRelaxed(std::move(continuable), context);

    if (sync_result.is_value())
    {
        return std::move(sync_result).get_value();
    }
    else
    {
        if (cti::exception_t e = sync_result.get_exception())
        {
            std::rethrow_exception(e);
        }
        else
        {
            throw WaitOnContextCancelledException();
        }
    }
}
}  // namespace internal

inline auto waitOnContext(asio::io_context& context)
{
    return [&context](auto&& continuable)
    { return internal::waitOnContextAndUnpack(std::forward<decltype(continuable)>(continuable), context); };
}

template <typename T>
auto runContextModal(T&& asyncJob)
{
    asio::io_context context;
    return std::forward<T>(asyncJob)(context.get_executor()).apply(waitOnContext(context));
}
