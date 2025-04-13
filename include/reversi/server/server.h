#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "reversi/server/session.h"

namespace reversi::server {

using Pos = core::Pos;
using TurnResult = core::TurnResult;

template <typename UserId>
class Server {
 public:
  using SessionT = Session<UserId>;

  SessionId CreateGame(UserId user_id) {
    std::lock_guard lock(mutex_);
    auto session_id = next_session_id_;
    ++next_session_id_;
    sessions_.insert({session_id, Session(Game(), user_id, session_id)});
    players_.insert({user_id, session_id});
    return session_id;
  }
  std::optional<SessionT*> TryConnectToGame(UserId user_id,
                                            SessionId session_id) {
    auto session_opt = FindSessionById(session_id);
    if (!session_opt) {
      return std::nullopt;
    }
    auto session = *session_opt;

    std::lock_guard lock(mutex_);
    if (session->TryConnect(user_id)) {
      players_.insert({user_id, session->GetGameId()});
      return session;
    }
    return std::nullopt;
  }
  TurnResult MakeTurn(UserId user_id, SessionT* session, Pos pos) {
    std::lock_guard lock(mutex_);
    auto game_opt = session->GetGame(user_id);
    if (!game_opt) {
      return TurnResult::kFailure;
    }
    auto* game = *game_opt;

    auto result = game->MakeTurn(pos);
    if (game->IsFinished()) {
      DeleteSession(session);
    }
    return result;
  }

  std::optional<SessionT*> FindSessionById(SessionId session_id) {
    std::shared_lock lock(mutex_);

    auto iter = sessions_.find(session_id);
    if (iter == sessions_.end()) {
      return std::nullopt;
    }
    return &iter->second;
  }

  std::optional<SessionT*> FindSessionByPlayerId(UserId user_id) {
    std::shared_lock lock(mutex_);

    auto session_iter = players_.find(user_id);
    if (session_iter == players_.end()) {
      return std::nullopt;
    }

    auto iter = sessions_.find(session_iter->second);
    if (iter == sessions_.end()) {
      return std::nullopt;
    }
    return &iter->second;
  }

  void DeleteSession(SessionT* session) {
    players_.erase(session->GetWhitePlayer());
    players_.erase(session->GetBlackPlayer());
    sessions_.erase(session->GetGameId());
  }

 private:
  std::unordered_map<SessionId, SessionT> sessions_;
  std::unordered_map<UserId, SessionId> players_;
  SessionId next_session_id_{};
  std::shared_mutex mutex_;
};

}  // namespace reversi::server
