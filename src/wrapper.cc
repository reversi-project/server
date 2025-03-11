#include <algorithm>
#include <optional>
#include <random>
#include <utility>

#include "wrapper.h"

namespace reversi::server {

namespace util {

static constexpr std::array kLetters = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
    'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

PlayerKey GenerateRandomPlayerKey() {
  std::default_random_engine rng(std::random_device{}());
  std::uniform_int_distribution<> dist(0, kLetters.size() - 1);
  auto rnd_char = [&dist, &rng]() { return kLetters[dist(rng)]; };

  std::string result(kPlayerKeyLength, 0);
  std::generate_n(result.begin(), kPlayerKeyLength, rnd_char);
  return result;
}

}  // namespace util

KeyPair KeyPair::GenerateRandom() {
  return {.white_key = util::GenerateRandomPlayerKey(),
          .black_key = util::GenerateRandomPlayerKey()};
}

Wrapper::Wrapper(Game game, KeyPair key_pair)
    : game_(game), key_pair_(std::move(key_pair)) {}

std::optional<PlayerKey> Wrapper::TryConnect() {
  if (state_ != GameState::kLoading) {
    return std::nullopt;
  }

  state_ = GameState::kPlaying;
  return std::make_optional(key_pair_.black_key);
}

std::optional<Game*> Wrapper::GetGame(const PlayerKey& key) {
  if (state_ != GameState::kPlaying) {
    return std::nullopt;
  }

  if (key == key_pair_.white_key &&
      game_.GetCurrentSide() == PlayerSide::kWhite) {
    return std::make_optional(&game_);
  }

  if (key == key_pair_.black_key &&
      game_.GetCurrentSide() == PlayerSide::kBlack) {
    return std::make_optional(&game_);
  }

  return std::nullopt;
}

}  // namespace reversi::server
