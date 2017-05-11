#include "chessboard.hh"

ChessBoard::ChessBoard()
{
  /*
  board_ = {
  {0x20, 0x3000, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90},
  {20, 30, 40, 50, 60, 70, 80, 90}
};
  */
}

void ChessBoard::print_board()
{
  for (unsigned i = 0; i < board_.size(); ++i)
  {
    for (unsigned j = 0; j < board_[0].size(); j++)
      std::cout << std::hex << std::setfill('0') << std::setw(2) << board_[i][j] << " ";
    std::cout << std::endl;
  }
}

ChessBoard::cell_t ChessBoard::get_square(plugin::Position position)
{
  int j = (int)position.file_get();
  int i = (int)position.rank_get();
  std::cout << std::hex << std::setfill('0') << std::setw(2) << board_[i][j] << " ";
    std::cout << std::endl;
  return board_[i][j];
}

ChessBoard::cell_t ChessBoard::get_opt(plugin::Position position, cell_t mask)
{
  return get_square(position) & mask;
}

bool ChessBoard::moved_at(plugin::Position position)
{
  return (bool)get_opt(position, 0b00001000);
}

bool ChessBoard::color_at(plugin::Position position)
{
  return (bool)get_opt(position, 0b10000000);
}

bool ChessBoard::castleflag_at(plugin::Position position)
{
  return (bool)get_opt(position, 0b00010000);
}

std::experimental::optional<plugin::PieceType>
ChessBoard::piece_type_at(plugin::Position position)
{
  cell_t type_b = get_opt(position, 0b00000111);
  if(type_b != 0b00000111)
    return plugin::piecetype_array()[type_b];
  return std::experimental::nullopt;
}
