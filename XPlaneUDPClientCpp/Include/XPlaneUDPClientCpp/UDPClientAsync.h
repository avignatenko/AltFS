#pragma once

#include <asio.hpp>
#include <continuable/continuable.hpp>

#include <thread>

namespace xplaneudpcpp
{
class UDPClient;

class UDPClientAsync
{
public:
    UDPClientAsync(asio::any_io_executor caller, const std::string& address, int port, int localPort);
    ~UDPClientAsync();

    cti::continuable<> connect();

    cti::continuable<> unsubscribeAll();

    cti::continuable<> writeDataref(const std::string& dataref, float f);

    cti::continuable<> subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);
    cti::continuable<> unsubscribeDataref(const std::string& dataref);

private:
    class AsioPostExecutor
    {
    public:
        AsioPostExecutor(asio::any_io_executor ex) : ex_(ex) {}

        template <class T>
        void operator()(T&& work)
        {
            asio::post(ex_, [work = std::forward<T>(work)]() mutable { std::move(work)(); });
        }

    private:
        asio::any_io_executor ex_;
    };

    AsioPostExecutor asioPostExecutor(asio::any_io_executor ex) { return AsioPostExecutor(ex); }

    template <typename E>
    struct ScheduleOn
    {
        explicit ScheduleOn(E&& e) : e(std::move(e)) {}

        template <typename... Args>
        constexpr auto operator()(Args&&... args)
        {
            auto cont = [e = std::forward<E>(e),
                         argsTuple = std::make_tuple(std::forward<Args>(args)...)](auto&& promise) mutable
            {
                std::move(e)(
                    [promise = std::forward<decltype(promise)>(promise), argsTuple = std::move(argsTuple)]() mutable
                    {
                        std::apply([promise = std::move(promise)](Args&&... argsOriginal) mutable
                                   { std::move(promise).set_value(std::move(argsOriginal)...); }, std::move(argsTuple));
                    });
            };

            return cti::detail::base::attorney::create_from(std::move(cont),
                                                            typename cti::detail::hints::from_args<Args...>::type{},
                                                            cti::detail::util::ownership{});
        }

        template <typename... Args>
        auto operator()(cti::exception_arg_t, cti::exception_t t)
        {
            auto cont = [e = std::forward<decltype(e)>(e), t](auto&& promise) mutable
            {
                std::move(e)([promise = std::forward<decltype(promise)>(promise), t]() mutable
                             { std::move(promise).set_exception(t); });
            };

            return cti::detail::base::attorney::create_from(std::move(cont),
                                                            typename cti::detail::hints::from_args<Args...>::type{},
                                                            cti::detail::util::ownership{});
        }

        E e;
    };

    ScheduleOn<AsioPostExecutor> postOnAsio(asio::any_io_executor ex) { return ScheduleOn(asioPostExecutor(ex)); }

    template <class FUNC>
    auto runAsync(FUNC&& f)
    {
        return cti::async_on(std::forward<FUNC>(f), asioPostExecutor(ctx_.get_executor())).next(postOnAsio(caller_));
    }

private:
    asio::any_io_executor caller_;
    asio::io_context ctx_;
    std::thread thread_;
    std::unique_ptr<UDPClient> client_;
};

}  // namespace xplaneudpcpp
