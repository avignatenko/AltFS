#pragma once

#include <functional>
#include <boost/asio.hpp>
#include <thread>

namespace xplaneudpcpp
{
    
    class BeaconListener
    {
    public:

         struct ServerInfo
         {
             int hostId;
             int version;
             std::string host;
             int port;
         };

        using Callback = std::function<bool(const ServerInfo&)> ;

        BeaconListener(Callback callback);
        ~BeaconListener();

    private:

        Callback m_callback;
        boost::asio::io_service io_service;
 
        std::unique_ptr<std::thread> m_thread;
    };


}
