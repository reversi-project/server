#include <optional>

#include "game_session.h"

namespace reversi::server {

GameSession::GameSession(Game game, WebSocket* ws, GameId game_id)
    : game_{game}, white_player_{ws}, game_id_{game_id} {}

bool GameSession::TryConnect(WebSocket* ws) {
  if (state_ != SessionState::kLoading) {
    return false;
  }
  if (white_player_ == ws) {
    return false;
  }

  state_ = SessionState::kPlaying;
  black_player_ = ws;
  return true;
}

std::optional<Game*> GameSession::GetGame(WebSocket* ws) {
  if (state_ != SessionState::kPlaying) {
    return std::nullopt;
  }

  if (ws == white_player_ && game_.GetCurrentSide() == PlayerSide::kWhite) {
    return std::make_optional(&game_);
  }

  if (ws == black_player_ && game_.GetCurrentSide() == PlayerSide::kBlack) {
    return std::make_optional(&game_);
  }

  return std::nullopt;
}

[[nodiscard]] GameId GameSession::GetGameId() const { return game_id_; }

[[nodiscard]] WebSocket* GameSession::GetWhitePlayer() const {
  return white_player_;
}

[[nodiscard]] WebSocket* GameSession::GetBlackPlayer() const {
  return black_player_;
}

WebSocket* GameSession::GetOpponent(WebSocket* ws) {
  if (ws == white_player_) {
    return black_player_;
  }
  if (ws == black_player_) {
    return white_player_;
  }
  return nullptr;
}

}  // namespace reversi::server
