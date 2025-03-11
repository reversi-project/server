#include <mutex>

#include "game.h"
#include "server.h"
#include "wrapper.h"

namespace reversi::server {

Credentials Server::CreateGame() {
  auto key_pair = KeyPair::GenerateRandom();
  auto wrapper = Wrapper(Game(), key_pair);

  std::unique_lock lock(mutex_);
  games_.insert({next_game_id_, std::move(wrapper)});
  Credentials result = {.key = key_pair.white_key, .game_id = next_game_id_};
  ++next_game_id_;
  return result;
}

std::optional<Credentials> Server::TryConnectToGame(GameId game_id) {
  std::unique_lock lock(mutex_);

  auto iter = games_.find(game_id);
  if (iter == games_.end()) {
    return std::nullopt;
  }
  auto& wrapper = iter->second;

  auto key_opt = wrapper.TryConnect();
  return key_opt.transform(
      [game_id](auto&& key) { return Credentials{key, game_id}; });
}

TurnResult Server::MakeTurn(GameId game_id, const PlayerKey& key, Pos pos) {
  std::unique_lock lock(mutex_);

  auto iter = games_.find(game_id);
  if (iter == games_.end()) {
    return TurnResult::kFailure;
  }
  auto& wrapper = iter->second;

  auto game_opt = wrapper.GetGame(key);
  if (!game_opt) {
    return TurnResult::kFailure;
  }
  auto* game = *game_opt;

  auto result = game->MakeTurn(pos);
  if (game->IsFinished()) {
    games_.erase(game_id);
  }
  return result;
}

}  // namespace reversi::server
