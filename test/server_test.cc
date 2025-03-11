#include <gtest/gtest.h>

#include "game.h"
#include "server.h"

using reversi::server::Pos;
using reversi::server::Server;
using reversi::server::TurnResult;

TEST(ServerSuite, CreateGame) {
  auto server = Server();

  auto credentials = server.CreateGame();

  ASSERT_EQ(0, credentials.game_id);
}

TEST(ServerSuite, ConnectToAvailableGame) {
  auto server = Server();
  auto credentials = server.CreateGame();

  auto result = server.TryConnectToGame(credentials.game_id);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(credentials.game_id, (*result).game_id);
}

TEST(ServerSuite, ConnectToNonExistingGame) {
  auto server = Server();

  auto result = server.TryConnectToGame(0);

  ASSERT_FALSE(result.has_value());
}

TEST(ServerSuite, ConnectToFullGame) {
  auto server = Server();
  auto credentials = server.CreateGame();

  server.TryConnectToGame(credentials.game_id);
  auto result = server.TryConnectToGame(credentials.game_id);

  ASSERT_FALSE(result.has_value());
}

TEST(ServerSuite, MakeValidTurn) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto second_player = server.TryConnectToGame(first_player.game_id);
  auto pos = Pos(4, 2);

  auto result = server.MakeTurn(first_player.game_id, first_player.key, pos);

  ASSERT_EQ(TurnResult::kSuccess, result);
}

TEST(ServerSuite, MakeInvalidTurn) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto second_player = server.TryConnectToGame(first_player.game_id);
  auto pos = Pos(0, 0);

  auto result = server.MakeTurn(first_player.game_id, first_player.key, pos);

  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(ServerSuite, MakeTurnInUnknownGame) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto second_player = server.TryConnectToGame(first_player.game_id);
  auto pos = Pos(4, 2);

  auto result = server.MakeTurn(2, first_player.key, pos);

  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(ServerSuite, MakeTurnWithInvalidKey) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto second_player = server.TryConnectToGame(first_player.game_id);
  auto pos = Pos(4, 2);

  auto result = server.MakeTurn(first_player.game_id, "invalid_key", pos);

  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(ServerSuite, MakeTurnInInvalidOrder) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto second_player = server.TryConnectToGame(first_player.game_id).value();
  auto pos = Pos(4, 2);

  auto result = server.MakeTurn(second_player.game_id, second_player.key, pos);

  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(ServerSuite, MakeTurnInNotStartedGame) {
  auto server = Server();
  auto first_player = server.CreateGame();
  auto pos = Pos(4, 2);

  auto result = server.MakeTurn(first_player.game_id, first_player.key, pos);

  ASSERT_EQ(TurnResult::kFailure, result);
}
