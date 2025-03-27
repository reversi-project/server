#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "game.h"

namespace reversi::server {

namespace beast = boost::beast;
namespace ws = beast::websocket;

using WebSocket = ws::stream<beast::tcp_stream>;

enum class SessionState : uint8_t {
  kLoading,
  kPlaying,
};

class GameSession {
 public:
  explicit GameSession(Game game, WebSocket* ws, GameId game_id);

  bool TryConnect(WebSocket* ws);
  std::optional<Game*> GetGame(WebSocket* ws);
  [[nodiscard]] GameId GetGameId() const;
  [[nodiscard]] WebSocket* GetWhitePlayer() const;
  [[nodiscard]] WebSocket* GetBlackPlayer() const;
  WebSocket* GetOpponent(WebSocket* ws);

 private:
  Game game_;
  GameId game_id_;
  SessionState state_{SessionState::kLoading};
  WebSocket* white_player_;
  WebSocket* black_player_{nullptr};
};

}  // namespace reversi::server
