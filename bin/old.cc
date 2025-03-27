// #include <reversi/core/board.h>
// #include <reversi/core/player.h>

// #include <iostream>
// #include <ostream>

// using reversi::core::Board;
// using reversi::core::BoardIdxT;
// using reversi::core::BoardItem;
// using reversi::core::kBoardSize;
// using reversi::core::Player;
// using reversi::core::PlayerSide;
// using reversi::core::Pos;
// using reversi::core::TurnResult;

// namespace {

// std::ostream& operator<<(std::ostream& stream, BoardItem item) {
//   using reversi::core::BoardItem;

//   switch (item) {
//     case BoardItem::kEmpty:
//       return stream << " ";
//     case BoardItem::kWhite:
//       return stream << "W";
//     case BoardItem::kBlack:
//       return stream << "B";
//   }
// }

// void MakeLine(std::ostream& stream, std::size_t length) {
//   stream << std::string(length + 2, '-') << "\n";
// }

// std::ostream& operator<<(std::ostream& stream, const Board& board) {
//   stream << "  ";
//   for (BoardIdxT j = 0; j < kBoardSize; ++j) {
//     stream << "| " << j << " ";
//   }
//   stream << "|\n";

//   for (BoardIdxT i = 0; i < kBoardSize; ++i) {
//     MakeLine(stream, (kBoardSize * 4) + 1);
//     stream << i << " ";
//     for (BoardIdxT j = 0; j < kBoardSize; ++j) {
//       stream << "| " << board.Get({.row = i, .col = j}) << " ";
//     }
//     stream << "|\n";
//   }
//   MakeLine(stream, (kBoardSize * 4) + 1);

//   return stream;
// }

// bool ExecuteTurn(const Board& board, Player& player, const char* title) {
//   if (!player.HaveTurns()) {
//     return false;
//   }
//   Pos pos = {.row = -1, .col = -1};

//   do {
//     std::cout << board << "\n";
//     std::cout << "Current player: " << title << "\n";
//     std::cout << "Enter pos: \n";
//     std::cin >> pos.row >> pos.col;
//   } while (player.MakeTurn(pos) != TurnResult::kSuccess);

//   return true;
// }

// }  // namespace

// int main() {
//   auto board = Board::Create();
//   auto first_player = Player(board, PlayerSide::kWhite);
//   auto second_player = Player(board, PlayerSide::kBlack);

//   while (ExecuteTurn(board, first_player, "first (W)") &&
//          ExecuteTurn(board, second_player, "second (B)")) {
//   }

//   std::cout << "Game finished:\n";
//   std::cout << board << "\n";
// }

// #include <iostream>

// #include "server.h"

// using reversi::server::Pos;
// using reversi::server::Server;

// int main() {
//   auto server = Server();
//   auto first_player = server.CreateGame();
//   auto second_player = server.TryConnectToGame(first_player.game_id);
//   auto pos = Pos(4, 2);

//   auto result = server.MakeTurn(first_player.game_id, first_player.key, pos);

//   std::cout << static_cast<int>(result) << "\n";
// }
