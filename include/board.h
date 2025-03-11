#pragma once

#include <array>
#include <cstdint>

namespace reversi::server {

using BoardIdxT = int;

static constexpr BoardIdxT kBoardSize = 8;

struct Direction {
  BoardIdxT row_step;
  BoardIdxT col_step;
};

struct Pos {
  BoardIdxT row;
  BoardIdxT col;

  [[nodiscard]] bool InsideBoard() const noexcept {
    return row >= 0 && row < kBoardSize && col >= 0 && col < kBoardSize;
  }

  [[nodiscard]] Pos Move(Direction direction) const noexcept {
    return {.row = row + direction.row_step, .col = col + direction.col_step};
  }
};

inline bool operator==(const Pos& lhs, const Pos& rhs) noexcept {
  return lhs.row == rhs.row && lhs.col == rhs.col;
}

inline bool operator!=(const Pos& lhs, const Pos& rhs) noexcept {
  return !(lhs == rhs);
}

enum class BoardItem : std::uint8_t {
  kEmpty,
  kWhite,
  kBlack,
};

class Board {
 public:
  static Board Initial() noexcept {
    Board board;

    const auto high = kBoardSize / 2;
    const auto low = high - 1;

    board.items_[low][low] = BoardItem::kWhite;
    board.items_[low][high] = BoardItem::kBlack;
    board.items_[high][low] = BoardItem::kBlack;
    board.items_[high][high] = BoardItem::kWhite;

    return board;
  }

  [[nodiscard]] BoardItem Get(Pos pos) const {
    return items_[pos.row][pos.col];
  }

  void Set(Pos pos, BoardItem item) { items_[pos.row][pos.col] = item; }

 private:
  std::array<std::array<BoardItem, kBoardSize>, kBoardSize> items_{};

  friend bool operator==(const Board& lhs, const Board& rhs);
};

inline bool operator==(const Board& lhs, const Board& rhs) {
  return lhs.items_ == rhs.items_;
}

inline bool operator!=(const Board& lhs, const Board& rhs) {
  return !(lhs == rhs);
}

}  // namespace reversi::server
