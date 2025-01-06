#include <XPlaneUDPClientCpp/BeaconListener.h>
#include <catch2/catch_test_macros.hpp>

#include <asio/io_context.hpp>

TEST_CASE("Basic discovery test", "[BeaconListener]")
{
    asio::io_context io;
    bool discovered = false;
    xplaneudpcpp::BeaconListener::getXPlaneServerBroadcastAsync(io.get_executor())
        .then([&discovered](xplaneudpcpp::BeaconListener::ServerInfo info) { discovered = true; });

    io.run_for(std::chrono::seconds(10));

    REQUIRE(discovered);
}
