#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <shared_mutex>
#include <unordered_map>

#include "game.h"
#include "game_session.h"

namespace reversi::server {

using RequestType = const std::string&;
using BeastErrorCode = const beast::error_code&;

class Server {
 public:
  using Callback = std::function<void(WebSocket*, const std::string& message)>;

  void HandleRequest(RequestType req, WebSocket* ws, Callback cb);
  bool HandleError(BeastErrorCode ec, WebSocket* ws, Callback cb);

 private:
  std::unordered_map<GameId, GameSession> games_;
  std::unordered_map<WebSocket*, GameId> players_;
  GameId next_game_id_;
  std::shared_mutex mutex_;

  void HandleCreateGame(std::stringstream& /*input*/, WebSocket* ws,
                        Callback cb);
  void HandleConnect(std::stringstream& input, WebSocket* ws, Callback cb);
  void HandleTurn(std::stringstream& input, WebSocket* ws, Callback cb);

  GameId CreateGame(WebSocket* ws);
  std::optional<GameSession*> TryConnectToGame(WebSocket* ws, GameId game_id);
  TurnResult MakeTurn(WebSocket* ws, GameSession* session, Pos pos);

  std::optional<GameSession*> FindSessionById(GameId game_id);
};

using ServerPtr = std::shared_ptr<Server>;

}  // namespace reversi::server
