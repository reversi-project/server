#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

#include "reversi/contract/common.h"
#include "reversi/contract/request.h"
#include "reversi/contract/response.h"
#include "reversi/server/server.h"

namespace reversi::server {

using RequestRaw = contract::RequestRaw;
using ResponseRaw = contract::ResponseRaw;

using BeastErrorCode = const boost::beast::error_code&;

template <typename UserId>
struct Action {
  UserId user_id;
  contract::Response res;
};

template <typename UserId>
using ActionList = std::vector<Action<UserId>>;

template <typename UserId>
class Handler {
 public:
  ActionList<UserId> HandleRequest(UserId user_id, const RequestRaw& raw) {
    auto req_exp = contract::RequestFromRaw(raw);
    if (!req_exp) {
      return {{user_id, contract::ErrorResponse(req_exp.error())}};
    }
    auto req = req_exp.value();

    const auto visitor = contract::Overloads{
        [this, user_id](
            const contract::CreateRequest& /*req*/) -> ActionList<UserId> {
          auto game_id = server_.CreateGame(user_id);
          return {{user_id, contract::GameCreatedResponse(game_id)}};
        },
        [this,
         user_id](const contract::ConnectRequest& req) -> ActionList<UserId> {
          auto session_opt = server_.TryConnectToGame(user_id, req.game_id);
          if (session_opt) {
            auto opponent_id = (*session_opt)->GetOpponent(user_id);
            return {{user_id, contract::ConnectedResponse{req.game_id}},
                    {opponent_id, contract::GameStartResponse{}}};
          }
          return {
              {user_id, contract::ErrorResponse("Unable to connect to game")}};
        },
        [this,
         user_id](const contract::TurnRequest& req) -> ActionList<UserId> {
          auto session_opt = server_.FindSessionById(req.game_id);
          if (!session_opt) {
            return {
                {user_id, contract::ErrorResponse("Game session not found")}};
          }
          auto session = *session_opt;

          auto turn_result = server_.MakeTurn(user_id, session, req.pos);
          if (turn_result == TurnResult::kFailure) {
            return {{user_id, contract::FailureTurnResponse{}}};
          }
          auto opponent_id = session->GetOpponent(user_id);
          return {{user_id, contract::SuccessTurnResponse{}},
                  {opponent_id, contract::OpponentTurnResponse{req.pos}}};
        },
    };
    return std::visit(visitor, req);
  }

  std::optional<ActionList<UserId>> HandleError(UserId user_id,
                                                BeastErrorCode ec) {
    if (!ec) {
      return std::nullopt;
    }
    std::cerr << "[WARN] " << ec.message() << '\n';

    auto result = ActionList<UserId>{};
    auto session_opt = server_.FindSessionByPlayerId(user_id);
    if (session_opt) {
      auto session = *session_opt;
      auto white_player = session->GetWhitePlayer();
      auto black_player = session->GetBlackPlayer();
      if (white_player && white_player != user_id) {
        result.push_back({white_player, contract::QuitResponse{}});
      }
      if (black_player && black_player != user_id) {
        result.push_back({black_player, contract::QuitResponse{}});
      }
      server_.DeleteSession(*session_opt);
    }
    return result;
  }

 private:
  Server<UserId> server_{};
};

}  // namespace reversi::server
