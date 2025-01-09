#pragma once

#include "ActiveObject.h"

#include <asio.hpp>
#include <continuable/continuable.hpp>

#include <thread>

namespace xplaneudpcpp
{
class UDPClient;

class UDPClientAsync : private ActiveObject
{
public:
    UDPClientAsync(asio::any_io_executor caller, const std::string& address, int port, int localPort);
    ~UDPClientAsync();

    cti::continuable<> connect();

    cti::continuable<> writeDataref(const std::string& dataref, float f);

    cti::continuable<> subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);

    cti::continuable<> unsubscribeDataref(const std::string& dataref);

    cti::continuable<> unsubscribeAll();

private:
    std::unique_ptr<UDPClient> client_;
};

}  // namespace xplaneudpcpp
