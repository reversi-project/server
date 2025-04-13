#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast.hpp>

#include "reversi/server/app.h"

using namespace std::string_literals;

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace ip = asio::ip;

using reversi::server::Endpoint;
using reversi::server::FlatBuffer;
using reversi::server::IoContext;
using reversi::server::PortT;
using reversi::server::RunApp;
using reversi::server::WebSocket;

using Resolver = ip::tcp::resolver;
static constexpr std::string kHost = "127.0.0.1";
static constexpr PortT kPort = 8080;
static constexpr auto kCreatedLength = 8;

class Client {
 public:
  explicit Client() : context_{IoContext{}}, ws_{context_} {
    ws_.set_option(
        ws::stream_base::timeout::suggested(beast::role_type::client));
    auto addr = ip::make_address(kHost);
    auto endpoint = Endpoint{addr, kPort};

    beast::get_lowest_layer(ws_).socket().connect(endpoint);
    ws_.handshake(fmt::format("{}:{}", kHost, kPort), "/");
  }

  std::string Read() {
    FlatBuffer buffer;
    ws_.read(buffer);
    return beast::buffers_to_string(buffer.data());
  }

  void Write(const std::string& message) { ws_.write(asio::buffer(message)); }

 private:
  IoContext context_;
  WebSocket ws_;
};

class ServerSuite : public ::testing::Test {
 protected:
  void SetUp() override {
    server_thread_ = std::thread([] { RunApp(kPort, 1); });
    constexpr auto kTimeout = 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(kTimeout));
  }

  void TearDown() override { server_thread_.detach(); }

 private:
  std::thread server_thread_;
};

TEST_F(ServerSuite, CreateGame) {
  auto client = Client{};

  client.Write("create");

  ASSERT_TRUE(client.Read().starts_with("created"));
}

TEST_F(ServerSuite, ValidConnectGame) {
  auto client1 = Client{};
  auto client2 = Client{};

  client1.Write("create");
  auto game_id = client1.Read().substr(kCreatedLength);

  client2.Write(fmt::format("connect {}", game_id));

  ASSERT_EQ(client2.Read(), "connected");
  ASSERT_EQ(client1.Read(), "start");
}

TEST_F(ServerSuite, InvalidConnectGame) {
  auto client = Client{};

  client.Write("connect");

  ASSERT_EQ(client.Read(), "error ConnectRequest: expect 1 args but got 0"s);
}

TEST_F(ServerSuite, InvalidRequest) {
  auto client = Client{};

  client.Write("connect not_int");

  ASSERT_EQ(client.Read(), "error ConnectRequest: unable to parse int"s);
}

TEST_F(ServerSuite, InvalidRequestArgs) {
  auto client = Client{};

  client.Write("connect 42 42");

  ASSERT_EQ(client.Read(), "error ConnectRequest: expect 1 args but got 2"s);
}

TEST_F(ServerSuite, UnknownRequest) {
  auto client = Client{};

  client.Write("unknown");

  ASSERT_EQ(client.Read(), "error Unknown request name: unknown"s);
}

TEST_F(ServerSuite, ValidTurn) {
  auto client1 = Client{};
  auto client2 = Client{};

  client1.Write("create");
  auto game_id = client1.Read().substr(kCreatedLength);

  client2.Write(fmt::format("connect {}", game_id));
  client2.Read();
  client1.Read();
  client1.Write(fmt::format("turn {} 4 2", game_id));

  ASSERT_EQ(client1.Read(), "ok");
  ASSERT_EQ(client2.Read(), "opponent 4 2");

  client2.Write(fmt::format("turn {} 5 2", game_id));

  ASSERT_EQ(client2.Read(), "ok");
  ASSERT_EQ(client1.Read(), "opponent 5 2");
}

TEST_F(ServerSuite, InvalidTurnOutsideBoard) {
  auto client1 = Client{};
  auto client2 = Client{};

  client1.Write("create");
  auto res = client1.Read();
  auto game_id = res.substr(kCreatedLength);

  client2.Write(fmt::format("connect {}", game_id));
  client2.Read();
  client1.Read();
  client1.Write(fmt::format("turn {} 100 200", game_id));

  ASSERT_EQ(client1.Read(), "fail");
}

TEST_F(ServerSuite, InvalidTurnNotEmpty) {
  auto client1 = Client{};
  auto client2 = Client{};

  client1.Write("create");
  auto res = client1.Read();
  auto game_id = res.substr(kCreatedLength);

  client2.Write(fmt::format("connect {}", game_id));
  client2.Read();
  client1.Read();

  client1.Write(fmt::format("turn {} 3 3", game_id));
  ASSERT_EQ(client1.Read(), "fail");
}
