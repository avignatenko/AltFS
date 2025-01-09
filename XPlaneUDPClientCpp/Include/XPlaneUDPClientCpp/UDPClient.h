#pragma once

#include <continuable/continuable.hpp>

#include <asio.hpp>

#include <array>
#include <functional>
#include <string>
#include <thread>

namespace xplaneudpcpp
{

class ClientReceiver;
class ClientSender;

// This client will use ex context to run it's operations and call callbacks
class UDPClient
{
public:
    UDPClient(asio::io_context& ex, const std::string& address, int port, int localPort);
    ~UDPClient();

    cti::continuable<> connect();

    cti::continuable<> unsubscribeAll();

    cti::continuable<> writeDataref(const std::string& dataref, float f);

    cti::continuable<> subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);
    cti::continuable<> unsubscribeDataref(const std::string& dataref);

private:
    asio::io_context& ex_;
    asio::ip::udp::socket socket_;
    std::shared_ptr<ClientSender> m_clientSender;
    std::shared_ptr<ClientReceiver> m_clientReceiver;
};

}  // namespace xplaneudpcpp
