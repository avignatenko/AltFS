#include <XPlaneUDPClientCpp/BeaconListener.h>
#include <XPlaneUDPClientCpp/UDPClient.h>
#include <catch2/catch_test_macros.hpp>

#include <spdlog/spdlog.h>
#include <asio/io_context.hpp>

TEST_CASE("Simple datadaref test", "[UDPClient]")
{
    asio::io_context io;
    auto work = asio::make_work_guard(io);

    std::unique_ptr<xplaneudpcpp::UDPClient> client;

    xplaneudpcpp::BeaconListener::getXPlaneServerBroadcastAsync(io.get_executor())
        .then(
            [&client, &io](xplaneudpcpp::BeaconListener::ServerInfo info)
            {
                client = std::make_unique<xplaneudpcpp::UDPClient>(io.get_executor(), info.host, info.port, 0);
                return client->connect();
            })
        .then(
            [&client]()
            {
                client->subscribeDataref("sim/flightmodel/position/latitude", 1,
                                         [](float f) { spdlog::info("Latitude: {}", f); });
            });

    io.run_for(std::chrono::seconds(20));
}
