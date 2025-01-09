#include "../Include/XPlaneUDPClientCpp/UDPClientAsync.h"
#include "../Include/XPlaneUDPClientCpp/UDPClient.h"

#include <spdlog/spdlog.h>
#include <continuable/continuable-transforms.hpp>

using namespace xplaneudpcpp;

UDPClientAsync::UDPClientAsync(asio::any_io_executor caller, const std::string& address, int port, int localPort)
    : ActiveObject(caller)
{
    runAsync([this, address, port, localPort]
             { client_ = std::make_unique<UDPClient>(context(), address, port, localPort); });
}

UDPClientAsync::~UDPClientAsync()
{
    runAsync([this] { client_.reset(); });
}

cti::continuable<> UDPClientAsync::connect()
{
    return runAsync([this] { return client_->connect(); });
}

cti::continuable<> UDPClientAsync::unsubscribeAll()
{
    return runAsync([this] { return client_->unsubscribeAll(); });
}
cti::continuable<> UDPClientAsync::writeDataref(const std::string& dataref, float f)
{
    return runAsync([this, dataref, f] { return client_->writeDataref(dataref, f); });
}

cti::continuable<> UDPClientAsync::subscribeDataref(const std::string& dataref, int freq,
                                                    std::function<void(float)> callback)
{
    return runAsync(
        [this, dataref, freq, callback]
        {
            auto repostCallback = [callback, caller = caller()](float value)
            { asio::post(caller, [callback, value] { callback(value); }); };

            return client_->subscribeDataref(dataref, freq, repostCallback);
        });
}

cti::continuable<> UDPClientAsync::unsubscribeDataref(const std::string& dataref)
{
    return runAsync([this, dataref] { return client_->unsubscribeDataref(dataref); });
}
