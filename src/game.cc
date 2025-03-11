#include <game.h>

#include <algorithm>

#include "board.h"

namespace reversi::server {

namespace {

PlayerSide Opposite(PlayerSide side) {
  if (side == PlayerSide::kBlack) {
    return PlayerSide::kWhite;
  }
  return PlayerSide::kBlack;
}

BoardItem GetSideBoardItem(PlayerSide side) {
  switch (side) {
    case PlayerSide::kWhite:
      return BoardItem::kWhite;
    case PlayerSide::kBlack:
      return BoardItem::kBlack;
  }
}

}  // namespace

Game::Game() : Game(Board::Initial(), PlayerSide::kWhite) {}

Game::Game(Board board, PlayerSide side) : board_(board), current_side_(side) {}

TurnResult Game::MakeTurn(Pos pos) {
  if (!pos.InsideBoard() || board_.Get(pos) != BoardItem::kEmpty) {
    return TurnResult::kFailure;
  }

  auto result = TurnResult::kFailure;
  for (const auto& direction : kDirections) {
    if (ExploreDirection(pos, direction)) {
      result = TurnResult::kSuccess;
    }
  }

  if (result == TurnResult::kSuccess) {
    current_side_ = Opposite(current_side_);
  }
  return result;
}

bool Game::IsFinished() const {
  for (BoardIdxT i = 0; i < kBoardSize; ++i) {
    for (BoardIdxT j = 0; j < kBoardSize; ++j) {
      if (CanMakeTurn({.row = i, .col = j})) {
        return false;
      }
    }
  }
  return true;
}

const Board& Game::GetBoard() const { return board_; }

PlayerSide Game::GetCurrentSide() const { return current_side_; }

bool Game::CanMakeTurn(Pos pos) const {
  if (!pos.InsideBoard() || board_.Get(pos) != BoardItem::kEmpty) {
    return false;
  }

  return std::ranges::any_of(kDirections, [this, pos](auto&& direction) {
    return TryMakeLine(pos, direction).has_value();
  });
}

std::optional<Pos> Game::TryMakeLine(Pos pos, Direction direction) const {
  auto self_item = GetSideBoardItem(current_side_);
  auto opposite_item = GetSideBoardItem(Opposite(current_side_));
  bool is_moved = false;

  pos = pos.Move(direction);
  while (pos.InsideBoard() && board_.Get(pos) == opposite_item) {
    pos = pos.Move(direction);
    is_moved = true;
  }

  if (pos.InsideBoard() && board_.Get(pos) == self_item && is_moved) {
    return std::make_optional(pos);
  }
  return std::nullopt;
}

bool Game::ExploreDirection(Pos pos, Direction direction) {
  auto head_opt = TryMakeLine(pos, direction);
  if (!head_opt) {
    return false;
  }

  auto head = *head_opt;
  auto self_item = GetSideBoardItem(current_side_);

  while (pos != head) {
    board_.Set(pos, self_item);
    pos = pos.Move(direction);
  }

  return true;
}

}  // namespace reversi::server
