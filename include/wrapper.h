#pragma once

#include <string>

#include "game.h"

namespace reversi::server {

using PlayerKey = std::string;
static constexpr std::size_t kPlayerKeyLength = 32;

namespace util {

PlayerKey GenerateRandomPlayerKey();

}  // namespace util

struct KeyPair {
  PlayerKey white_key;
  PlayerKey black_key;

  static KeyPair GenerateRandom();
};

enum class GameState : uint8_t {
  kLoading,
  kPlaying,
};

class Wrapper {
 public:
  explicit Wrapper(Game game, KeyPair key_pair);

  std::optional<PlayerKey> TryConnect();
  std::optional<Game*> GetGame(const PlayerKey& key);

 private:
  Game game_;
  GameState state_{GameState::kLoading};
  KeyPair key_pair_;
};

}  // namespace reversi::server
