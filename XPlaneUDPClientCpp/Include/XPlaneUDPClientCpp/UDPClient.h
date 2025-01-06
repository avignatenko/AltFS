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

class UDPClient
{
public:
    UDPClient(asio::any_io_executor ex, const std::string& address, int port, int16_t baseId);
    ~UDPClient();

    cti::continuable<> connect();

    void writeDataref(const std::string& dataref, float f);

    void subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);
    void unsubscribeDataref(const std::string& dataref);

private:
    asio::io_service io_;

    asio::ip::udp::socket socket_;
    std::unique_ptr<ClientSender> m_clientSender;
    std::unique_ptr<ClientReceiver> m_clientReceiver;
    std::unique_ptr<std::thread> m_thread;
};
}  // namespace xplaneudpcpp
