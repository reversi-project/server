#pragma once

#include <board.h>

#include <cstdint>
#include <optional>

namespace reversi::server {

using GameId = uint64_t;

enum class PlayerSide : std::uint8_t {
  kWhite,
  kBlack,
};

enum class TurnResult : std::uint8_t {
  kFailure,
  kSuccess,
};

class Game {
 public:
  explicit Game();
  explicit Game(Board board, PlayerSide side);

  TurnResult MakeTurn(Pos pos);
  [[nodiscard]] bool IsFinished() const;
  [[nodiscard]] const Board& GetBoard() const;
  [[nodiscard]] PlayerSide GetCurrentSide() const;

 private:
  Board board_;
  PlayerSide current_side_{PlayerSide::kWhite};

  static constexpr const std::array<Direction, 8> kDirections = {{
      {.row_step = +1, .col_step = +1},
      {.row_step = +1, .col_step = +0},
      {.row_step = +1, .col_step = -1},
      {.row_step = +0, .col_step = +1},
      {.row_step = +0, .col_step = -1},
      {.row_step = -1, .col_step = +1},
      {.row_step = -1, .col_step = +0},
      {.row_step = -1, .col_step = -1},
  }};

  [[nodiscard]] bool CanMakeTurn(Pos pos) const;
  [[nodiscard]] std::optional<Pos> TryMakeLine(Pos pos,
                                               Direction direction) const;
  bool ExploreDirection(Pos pos, Direction direction);
};

}  // namespace reversi::server
