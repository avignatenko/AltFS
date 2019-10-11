#pragma once

#include "promise.hpp"

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
             int hostId = 0;
             int version = 0;
             std::string host;
             int port = 0;
         };

        BeaconListener();
        ~BeaconListener();

        /// Return ServerInfo as value
        promise::Defer getXPlaneServerBroadcast();

    private:

        boost::asio::io_service io_service;
 
        std::unique_ptr<std::thread> m_thread;
    };


}
