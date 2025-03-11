#pragma once

#include <mutex>
#include <unordered_map>

#include "game.h"
#include "wrapper.h"

namespace reversi::server {

struct Credentials {
  PlayerKey key;
  GameId game_id;
};

class Server {
 public:
  Credentials CreateGame();
  std::optional<Credentials> TryConnectToGame(GameId game_id);
  TurnResult MakeTurn(GameId game_id, const PlayerKey& key, Pos pos);

 private:
  std::unordered_map<GameId, Wrapper> games_;
  GameId next_game_id_;
  std::mutex mutex_;
};

}  // namespace reversi::server
