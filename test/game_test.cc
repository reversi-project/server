#include <gtest/gtest.h>

#include "board.h"
#include "game.h"

using reversi::server::Board;
using reversi::server::BoardItem;
using reversi::server::Game;
using reversi::server::kBoardSize;
using reversi::server::PlayerSide;
using reversi::server::TurnResult;

namespace {

BoardItem MakeBoardItemFromRaw(int val) {
  switch (val) {
    case 0:
      return BoardItem::kEmpty;
    case 1:
      return BoardItem::kWhite;
    case 2:
      return BoardItem::kBlack;
    default:
      // todo
      return BoardItem::kEmpty;
  }
}

Board MakeBoardFromRaw(
    const std::array<std::array<int, kBoardSize>, kBoardSize>& raw) {
  auto board = Board::Initial();

  for (auto i = 0; i < kBoardSize; ++i) {
    for (auto j = 0; j < kBoardSize; ++j) {
      board.Set({.row = i, .col = j}, MakeBoardItemFromRaw(raw[i][j]));
    }
  }

  return board;
}

}  // namespace

TEST(GameSuite, InitialBoard) {
  auto board = Board::Initial();

  auto new_board = MakeBoardFromRaw({{
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 2, 0, 0, 0},
      {0, 0, 0, 2, 1, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  }});
  ASSERT_EQ(new_board, board);
}

TEST(GameSuite, OutsideBoardTurn) {
  auto game = Game();

  auto result = game.MakeTurn({.row = -1, .col = 1});

  auto new_board = MakeBoardFromRaw({{
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 2, 0, 0, 0},
      {0, 0, 0, 2, 1, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  }});
  ASSERT_EQ(new_board, game.GetBoard());
  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(GameSuite, CorrectTurn) {
  auto game = Game();

  auto result = game.MakeTurn({.row = 4, .col = 2});

  auto new_board = MakeBoardFromRaw({{
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 2, 0, 0, 0},
      {0, 0, 1, 1, 1, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  }});
  ASSERT_EQ(new_board, game.GetBoard());
  ASSERT_EQ(TurnResult::kSuccess, result);
}

TEST(GameSuite, IncorrectTurn) {
  auto game = Game();

  auto result = game.MakeTurn({.row = 2, .col = 2});

  auto new_board = MakeBoardFromRaw({{
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 2, 0, 0, 0},
      {0, 0, 0, 2, 1, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  }});
  ASSERT_EQ(new_board, game.GetBoard());
  ASSERT_EQ(TurnResult::kFailure, result);
}

TEST(GameSuite, PlayerHaveTurns) {
  auto game = Game();

  ASSERT_FALSE(game.IsFinished());
}

TEST(GameSuite, PlayerHaveNoTurnsBecauseNoEmptySquares) {
  auto board = MakeBoardFromRaw({{
      {1, 1, 1, 1, 1, 2, 2, 2},
      {2, 1, 2, 2, 2, 1, 1, 2},
      {1, 1, 1, 1, 1, 1, 1, 2},
      {2, 1, 1, 1, 2, 1, 1, 1},
      {1, 2, 1, 2, 1, 1, 1, 1},
      {1, 2, 1, 1, 1, 2, 2, 1},
      {1, 2, 1, 1, 2, 1, 1, 1},
      {1, 1, 1, 2, 1, 2, 1, 1},
  }});
  auto game = Game(board, PlayerSide::kWhite);

  ASSERT_TRUE(game.IsFinished());
}

TEST(GameSuite, PlayerHaveNoTurnsByRules) {
  auto board = MakeBoardFromRaw({{
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 2, 2, 0, 0, 0},
      {0, 0, 0, 2, 2, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  }});
  auto game = Game(board, PlayerSide::kBlack);

  ASSERT_TRUE(game.IsFinished());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
