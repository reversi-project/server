#include <gtest/gtest.h>

#include <cstdint>

#include "game_session.h"
#include "server.h"

using reversi::server::Server;
using reversi::server::WebSocket;
using namespace std::literals;

namespace {

using Responses = std::vector<std::pair<uint64_t, std::string>>;

class ServerWrapper {
 public:
  void MakeRequest(uint64_t ws, const std::string& req) {
    server_.HandleRequest(req, reinterpret_cast<WebSocket*>(ws),  // NOLINT
                          [this](WebSocket* ws, const std::string& message) {
                            responses_.emplace_back(
                                reinterpret_cast<uint64_t>(ws),  // NOLINT
                                message);
                          });
  }

  const Responses& GetResponses() const { return responses_; }

 private:
  Server server_;
  Responses responses_;
};

}  // namespace

TEST(ServerSuite, UnknownCommand) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "unknown");

  Responses expected = {{0, "error"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, CorrectCreate) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");

  Responses expected = {{0, "0"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, CorrectDoubleCreate) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(0, "create");

  Responses expected = {{0, "0"}, {0, "1"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, CorrectConnect) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 0");

  Responses expected = {{0, "0"}, {1, "connected"}, {0, "start"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, IncorrectConnect) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 222");

  Responses expected = {{0, "0"}, {1, "error"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, InccorectOrderOfTurns) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 0");
  server.MakeRequest(1, "turn 0 4 2");

  Responses expected = {{0, "0"}, {1, "connected"}, {0, "start"}, {1, "fail"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, IncorrectTurn) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 0");
  server.MakeRequest(0, "turn");

  Responses expected = {{0, "0"}, {1, "connected"}, {0, "start"}, {0, "error"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, IncorrectTurnWithoutGameId) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 0");
  server.MakeRequest(0, "turn 4 2");

  Responses expected = {{0, "0"}, {1, "connected"}, {0, "start"}, {0, "error"}};
  ASSERT_EQ(expected, server.GetResponses());
}

TEST(ServerSuite, CorrectTurn) {
  auto server = ServerWrapper{};

  server.MakeRequest(0, "create");
  server.MakeRequest(1, "connect 0");
  server.MakeRequest(0, "turn 0 4 2");

  Responses expected = {
      {0, "0"}, {1, "connected"}, {0, "start"}, {0, "ok"}, {1, "turn 4 2"}};
  ASSERT_EQ(expected, server.GetResponses());
}
