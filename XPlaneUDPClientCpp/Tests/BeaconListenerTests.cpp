#include <XPlaneUDPClientCpp/BeaconListener.h>
#include <catch2/catch_test_macros.hpp>

#include <asio/io_context.hpp>

TEST_CASE("Basic discovery test", "[BeaconListener]")
{
    asio::io_context io;
    auto work = asio::make_work_guard(io);

    xplaneudpcpp::BeaconListener::getXPlaneServerBroadcastAsync(io.get_executor())
        .then(
            [&work](xplaneudpcpp::BeaconListener::ServerInfo info)
            {
                REQUIRE(info.hostId == 1);
                REQUIRE(info.version == 1);
                REQUIRE(info.host == "192.168.0.01");
                REQUIRE(info.port == 49000);

                work.reset();
            });

    io.run();
}
