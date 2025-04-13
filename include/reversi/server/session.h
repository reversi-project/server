#pragma once

#include <optional>

#include "reversi/core/game.h"

namespace reversi::server {

using Game = core::Game;
using SessionId = core::GameId;
using PlayerSide = core::PlayerSide;

enum class SessionState : uint8_t {
  kLoading,
  kPlaying,
};

template <typename UserId>
class Session {
 public:
  explicit Session(Game game, UserId user_id, SessionId session_id)
      : game_{game}, white_player_{user_id}, session_id_{session_id} {}

  bool TryConnect(UserId user_id) {
    if (state_ != SessionState::kLoading) {
      return false;
    }
    if (white_player_ == user_id) {
      return false;
    }

    state_ = SessionState::kPlaying;
    black_player_ = user_id;
    return true;
  }

  std::optional<Game*> GetGame(UserId user_id) {
    if (state_ != SessionState::kPlaying) {
      return std::nullopt;
    }

    if (user_id == white_player_ &&
        game_.GetCurrentSide() == PlayerSide::kWhite) {
      return std::make_optional(&game_);
    }

    if (user_id == black_player_ &&
        game_.GetCurrentSide() == PlayerSide::kBlack) {
      return std::make_optional(&game_);
    }

    return std::nullopt;
  }

  [[nodiscard]] SessionId GetGameId() const { return session_id_; }

  [[nodiscard]] UserId GetWhitePlayer() const { return white_player_; }

  [[nodiscard]] UserId GetBlackPlayer() const { return black_player_; }

  UserId GetOpponent(UserId user_id) {
    if (user_id == white_player_) {
      return black_player_;
    }
    if (user_id == black_player_) {
      return white_player_;
    }
    return nullptr;
  }

 private:
  Game game_;
  SessionId session_id_;
  SessionState state_{SessionState::kLoading};
  UserId white_player_;
  UserId black_player_{nullptr};
};

}  // namespace reversi::server
