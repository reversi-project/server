#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>

#include "game_session.h"
#include "server.h"

namespace reversi::server {

namespace {

template <typename T>
std::optional<T> TryParseInt(std::stringstream& input) {
  std::string str;
  input >> str;
  auto value = T{};
  auto res = std::from_chars(str.data(), str.data() + str.size(), value);
  if (res.ec == std::errc{}) {
    return value;
  }
  return std::nullopt;
}

}  // namespace

void Server::HandleCreateGame(std::stringstream& /*input*/, WebSocket* ws,
                              Callback cb) {
  auto game_id = CreateGame(ws);
  std::stringstream out;
  out << game_id;
  cb(ws, out.str());
}

void Server::HandleConnect(std::stringstream& input, WebSocket* ws,
                           Callback cb) {
  const auto game_id_opt = TryParseInt<GameId>(input);
  auto session_opt = game_id_opt.and_then(
      [this, ws](auto&& game_id) { return TryConnectToGame(ws, game_id); });

  if (session_opt) {
    cb(ws, "connected");
    cb((*session_opt)->GetOpponent(ws), "start");
  } else {
    cb(ws, "error");
  }
}

void Server::HandleTurn(std::stringstream& input, WebSocket* ws, Callback cb) {
  auto game_id_opt = TryParseInt<GameId>(input);
  auto pos_x_opt = TryParseInt<BoardIdxT>(input);
  auto pos_y_opt = TryParseInt<BoardIdxT>(input);

  auto session_opt = game_id_opt.and_then(
      [this, ws](auto&& game_id) { return FindSessionById(game_id); });
  auto ows = session_opt.transform(
      [ws](auto&& session) { return session->GetOpponent(ws); });

  auto turn_result_opt = session_opt.and_then([&](auto&& session) {
    return pos_x_opt.and_then([&](auto&& pos_x) {
      return pos_y_opt.transform([&](auto&& pos_y) {
        const auto pos = Pos{pos_x, pos_y};
        return MakeTurn(ws, session, pos);
      });
    });
  });

  if (turn_result_opt) {
    if (*turn_result_opt == TurnResult::kFailure) {
      cb(ws, "fail");
    } else {
      cb(ws, "ok");

      std::stringstream out;
      out << "turn " << *pos_x_opt << " " << *pos_y_opt;
      cb(*ows, out.str());
    }
  } else {
    cb(ws, "error");
  }
}

void Server::HandleRequest(RequestType req, WebSocket* ws, Callback cb) {
  std::stringstream input(req);
  std::string command{};
  input >> command;

  if (command == "create") {
    HandleCreateGame(input, ws, cb);
  } else if (command == "connect") {
    HandleConnect(input, ws, cb);
  } else if (command == "turn") {
    HandleTurn(input, ws, cb);
  } else {
    cb(ws, "error");
  }
}

bool Server::HandleError(BeastErrorCode ec, WebSocket* ws, Callback cb) {
  if (ec) {
    std::lock_guard lock(mutex_);
    auto it = players_.find(ws);
    if (it != players_.end()) {
      auto game = games_.find(it->second);
      if (game != games_.end()) {
        const auto& white_player = game->second.GetWhitePlayer();
        const auto& black_player = game->second.GetBlackPlayer();

        if (white_player != ws) {
          cb(white_player, "quit");
        }
        if (black_player != ws) {
          cb(black_player, "quit");
        }

        players_.erase(white_player);
        players_.erase(black_player);
        games_.erase(game);
      }
    }
    std::cerr << "[WARN] " << ec.message() << '\n';
    return true;
  }
  return false;
}

GameId Server::CreateGame(WebSocket* ws) {
  {
    std::lock_guard lock(mutex_);
    games_.insert({next_game_id_, GameSession(Game(), ws, next_game_id_)});
    players_.insert({ws, next_game_id_});
  }
  auto result = next_game_id_;
  ++next_game_id_;
  return result;
}

std::optional<GameSession*> Server::TryConnectToGame(WebSocket* ws,
                                                     GameId game_id) {
  auto session_opt = FindSessionById(game_id);

  return session_opt.and_then(
      [this, ws](auto&& session) -> std::optional<GameSession*> {
        std::lock_guard lock(mutex_);
        if (session->TryConnect(ws)) {
          players_.insert({ws, session->GetGameId()});
          return session;
        }
        return std::nullopt;
      });
}

TurnResult Server::MakeTurn(WebSocket* ws, GameSession* session, Pos pos) {
  std::lock_guard lock(mutex_);

  auto game_opt = session->GetGame(ws);
  if (!game_opt) {
    return TurnResult::kFailure;
  }
  auto* game = *game_opt;

  auto result = game->MakeTurn(pos);
  if (game->IsFinished()) {
    players_.erase(session->GetWhitePlayer());
    players_.erase(session->GetBlackPlayer());
    games_.erase(session->GetGameId());
  }
  return result;
}

std::optional<GameSession*> Server::FindSessionById(GameId game_id) {
  std::shared_lock lock(mutex_);

  auto iter = games_.find(game_id);
  if (iter == games_.end()) {
    return std::nullopt;
  }
  return &iter->second;
}

}  // namespace reversi::server
