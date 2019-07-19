#pragma once

namespace xplaneudpcpp
{
    class UDPClient
    {
    public:

        UDPClient(const std::string& address, int port) {}

        void writeDataref(const std::string& dataref, float f) {}
        void subscribeDataref(const std::string& dataref, double freq, std::function<void(float)> callback) {}
    };
}