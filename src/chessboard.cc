#include "chessboard.hh"
#include "rule-checker.hh"
#include "piece/pawn.hh"
#include "piece/king.hh"
#include "piece/queen.hh"
#include "piece/bishop.hh"
#include "piece/knight.hh"
#include "piece/rook.hh"
#include "rule-checker.hh"

std::ostream& operator<<(std::ostream& o, const plugin::Position& p);
/*std::ostream& operator<<(std::ostream& o, const plugin::Position& p)
{
  o << (char)(static_cast<char>(p.file_get()) + 'A') << (char)(static_cast<char>(p.rank_get()) + '1');
  return o;
}*/

ChessBoard::ChessBoard(std::vector<plugin::Listener*> listeners)
  : listeners_(listeners)
{
}

int ChessBoard::update(Move& move) { // FIX ME PROMOTION
  if (!RuleChecker::is_move_valid(*this, move)) {
    for (auto l : listeners_)
      l->on_player_disqualified(move.color_get()); // Disqualified
    return -1;
  }
  //throw std::invalid_argument("invalid move - in ChessBoard update()");
  std::cerr << "Move is pseudo valid" << std::endl;

  plugin::PieceType piecetype_eaten;
  if (move.move_type_get() == Move::Type::QUIET)
  {
    const QuietMove& quiet_move = static_cast<const QuietMove&>(move);
    if (quiet_move.is_an_attack())
      piecetype_eaten = piecetype_get(quiet_move.end_get()).value();
  }


  apply_move(*this, move);
  if (RuleChecker::isCheck(*this, get_king_position(move.color_get()))) {
    for (auto l : listeners_)
      l->on_player_disqualified(move.color_get()); // Disqualified
    return -1;
  }
  std::cerr << "Move is completly valid" << std::endl;

  //throw std::invalid_argument("invalid move : The King would be in check");

  /* Update piece position*/
  if (move.move_type_get() == Move::Type::QUIET)
  {
    const QuietMove& quiet_move = static_cast<const QuietMove&>(move);
    /*move_piece(quiet_move.start_get(), quiet_move.end_get());
    std::cerr << "Moving piece" << std::endl;*/
    for (auto l : listeners_)
      l->on_piece_moved(quiet_move.piecetype_get(), quiet_move.start_get(), quiet_move.end_get());
    if (quiet_move.is_an_attack()) // Piece Taken
      for (auto l : listeners_)
        l->on_piece_taken(piecetype_eaten, quiet_move.end_get());
  }
  else /* Castling */
  {
    plugin::Position king_start_position = initial_king_position(move.color_get());
    plugin::Position king_end_position = castling_king_end_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING);
    plugin::Position rook_start_postion = initial_rook_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING);
    plugin::Position rook_end_position = castling_rook_end_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING);
    /*move_piece(king_start_position, king_end_position);
    move_piece(rook_start_postion, rook_end_position);*/

    for (auto l : listeners_)
      l->on_piece_moved(plugin::PieceType::KING, king_start_position, king_end_position);
    if (move.move_type_get() == Move::Type::KING_CASTLING)
      for (auto l : listeners_)
        l->on_kingside_castling(move.color_get());
    else
      for (auto l : listeners_)
        l->on_queenside_castling(move.color_get());

  }
  plugin::Color opponent_color = static_cast<plugin::Color>(not static_cast<bool>(move.color_get()));
  plugin::Position king_position = get_king_position(opponent_color);
  if (RuleChecker::isCheckmate(*this, king_position)) {
    for (auto l : listeners_)
      l->on_player_mat(move.color_get());
    return -1;
  }
  else if (RuleChecker::isCheck(*this, king_position)) {
    for (auto l : listeners_)
      l->on_player_check(move.color_get());
    return 0;
  }
  else if (RuleChecker::isStalemate(*this, opponent_color))
  {
    for (auto l : listeners_)
      l->on_player_pat(move.color_get());
    for (auto l : listeners_)
      l->on_draw();
    return -1;
  }
  /*else if (RuleChecker::is_a_draw()) {
    for (auto l : listeners_)
      l->on_draw();
    return -1;
  }*/

  /*if (RuleChecker::isCheckmate(*this, get_king_position(move.color_get())))
  {
    for (auto l : listeners_)
      l->on_player_check(move.color_get());
    //throw std::invalid_argument("CHECKMATE -- in chessboard update function");
  }*/
  return 0;
}

ChessBoard ChessBoard::apply_move(ChessBoard board, Move& move)
{
  if (move.move_type_get() == Move::Type::QUIET)
  {
    const QuietMove& quiet_move = static_cast<const QuietMove&>(move);
    move_piece(quiet_move.start_get(), quiet_move.end_get());
  }
  else /* Castling */
  {
    if (move.move_type_get() == Move::Type::KING_CASTLING)
    move_piece(initial_king_position(move.color_get()), castling_king_end_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING));
    move_piece(initial_rook_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING)
        , castling_rook_end_position(move.color_get(), move.move_type_get() == Move::Type::KING_CASTLING));
  }
  return board;
}

void ChessBoard::move_piece(plugin::Position start, plugin::Position end) {
  cell_t moving_piece = get_square(start);
  moving_piece |= 0b00001000;
  set_square(end, moving_piece);

  set_square(start, 0x7); //0b000001111
}

void ChessBoard::print() const
{
  for (int i = board_.size() - 1; i >= 0; --i)
  {
    for (unsigned j = 0; j < board_[0].size(); j++) {
      if (piecetype_get(plugin::Position((plugin::File)j,(plugin::Rank) i)) == std::experimental::nullopt)
        std::cerr << "_ ";
      else
        std::cerr << static_cast<char>(piecetype_get(plugin::Position((plugin::File)j,(plugin::Rank) i)).value()) << " ";

      //std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)board_[i][j] << " ";
    }
    std::cerr << std::endl;
  }
}

void ChessBoard::set_square(plugin::Position position, cell_t value) {
  board_[ 7 - static_cast<char>(position.rank_get())][static_cast<char>(position.file_get())] = value;
}

ChessBoard::cell_t ChessBoard::get_square(plugin::Position position) const
{
  char j = static_cast<char>(position.file_get());
  char i = 7 - static_cast<char>(position.rank_get());
  /*std::cout << "i = " << (unsigned)i << " j = " << (unsigned int)j << std::endl;
  std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)board_[i][j] << " ";
  std::cout << std::endl;*/
  return board_[i][j];
}

ChessBoard::cell_t ChessBoard::get_opt(plugin::Position position, cell_t mask) const
{
  return get_square(position) & mask;
}

bool ChessBoard::has_moved(plugin::Position position) const
{
  return (bool)get_opt(position, 0b00001000);
}

plugin::Color ChessBoard::color_get(plugin::Position position) const
{
  return static_cast<plugin::Color>(get_opt(position, 0b10000000) != 0);
}

bool ChessBoard::castleflag_get(plugin::Position position) const
{
  return (bool)get_opt(position, 0b00010000);
}

  std::experimental::optional<plugin::PieceType>
ChessBoard::piecetype_get(plugin::Position position) const
{
  cell_t type_b = get_opt(position, 0b00000111);
  if(type_b != 0b00000111)
    return plugin::piecetype_array()[type_b];
  return std::experimental::nullopt;
}

// Color : the color of the cell that is attacked
bool ChessBoard::is_attacked(plugin::Color color, plugin::Position current_cell) const
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
    {
      plugin::Position pos(static_cast<plugin::File>(i), static_cast<plugin::Rank>(j));
      if (piecetype_get(pos) != std::experimental::nullopt and color_get(pos) != color) {
        //std::cerr << " ATTACKER : " << pos << std::endl;
        try {
          if (RuleChecker::is_move_valid(*this,
            QuietMove(static_cast<plugin::Color>(not static_cast<bool>(color)),
            pos, current_cell, piecetype_get(pos).value(), true, false))) {
            return true;
          }
        }
        catch (std::invalid_argument& e) {
          return false;
        }
      }
    }
  return false;
}

std::vector<Move> ChessBoard::get_possible_actions(plugin::Position position)
{
  std::vector<Move> moves{};
  auto color_piece = color_get(position);
  auto piece_type = piecetype_get(position);
  if (piece_type == std::experimental::nullopt)
    return moves;

  switch (piece_type.value())
  {
    case plugin::PieceType::KING:
      for(int i = -1; i < 1; i++)
      {
        for(int j = -1; j < 1; j++)
          {
            if(i + static_cast<int>(position.file_get()) < 7
                && i + static_cast<int>(position.file_get()) > 0
                && j + static_cast<int>(position.rank_get()) < 7
                && j + static_cast<int>(position.rank_get()) > 0)
            {
              plugin::Position endpos1(static_cast<plugin::File>(i + static_cast<int>(0 + static_cast<int>(position.file_get()))), static_cast<plugin::Rank> (j + static_cast<int>(position.rank_get())));
              QuietMove move1(color_piece, position, endpos1, piece_type.value(),piecetype_get(endpos1) != std::experimental::nullopt , false);
              if(RuleChecker::isMoveAuthorized(*this, move1) && RuleChecker::isMoveLegal(*this,move1))
                moves.push_back(move1);
            }
          }
      }
      break;

    case plugin::PieceType::QUEEN:
      break;

    case plugin::PieceType::ROOK:
      for(int i = 0; i < 8; i++)
      {
        if(i + static_cast<int>(position.file_get()) < 7 )
        {
          plugin::Position endpos1(static_cast<plugin::File>(i + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (0 + static_cast<int>(position.rank_get())));
          QuietMove move1(color_piece, position, endpos1, piece_type.value(),piecetype_get(endpos1) != std::experimental::nullopt , false);
          if(RuleChecker::isMoveAuthorized(*this, move1) && RuleChecker::isMoveLegal(*this,move1))
              moves.push_back(move1);
        }

        if(i + static_cast<int>(position.rank_get()) < 7)
        {
          plugin::Position endpos2(static_cast<plugin::File>(0 + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (i + static_cast<int>(position.rank_get())));
          QuietMove move2(color_piece, position, endpos2, piece_type.value(), piecetype_get(endpos2)!= std::experimental::nullopt , false);
          if(RuleChecker::isMoveAuthorized(*this, move2) && RuleChecker::isMoveLegal(*this, move2))
            moves.push_back(move2);
        }
       }
      break;

    case plugin::PieceType::BISHOP:
      for(int j = -8; j < 8; j++)
      {
        if(j + static_cast<int>(position.rank_get()) < 7
           && j + static_cast<int>(position.rank_get()) > 0)
        {
          plugin::Position endpos1(static_cast<plugin::File>(j + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (j + static_cast<int>(position.rank_get())));
          QuietMove move1(color_piece, position, endpos1, piece_type.value(),piecetype_get(endpos1) != std::experimental::nullopt , false);
          if(RuleChecker::isMoveAuthorized(*this, move1) && RuleChecker::isMoveLegal(*this,move1))
            moves.push_back(move1);

          plugin::Position endpos2(static_cast<plugin::File>(-j + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (j + static_cast<int>(position.rank_get())));
          QuietMove move2(color_piece, position, endpos2, piece_type.value(), piecetype_get(endpos2)!= std::experimental::nullopt , false);
          if(RuleChecker::isMoveAuthorized(*this, move2) && RuleChecker::isMoveLegal(*this, move2))
            moves.push_back(move2);
        }
      }
      break;

    case plugin::PieceType::KNIGHT:
      for(int i = -2; i < 3; i++)
      {
        for(int j = -2; j < 3; j++)
        {
          if(i + static_cast<int>(position.file_get()) < 7
              && i + static_cast<int>(position.file_get()) > 0
              && j + static_cast<int>(position.rank_get()) < 7
              && j + static_cast<int>(position.rank_get()) > 0)
          {
            plugin::Position endpos1(static_cast<plugin::File>(i + static_cast<int>(0 + static_cast<int>(position.file_get()))) , static_cast<plugin::Rank> (j + static_cast<int>(position.rank_get())));
            QuietMove move1(color_piece, position, endpos1, piece_type.value(),piecetype_get(endpos1) != std::experimental::nullopt , false);
            if(RuleChecker::isMoveAuthorized(*this, move1) && RuleChecker::isMoveLegal(*this,move1))
              moves.push_back(move1);
          }
        }
      }
      break;

    case plugin::PieceType::PAWN:
      if(1 + static_cast<int>(position.rank_get()) < 7)
      {
        plugin::Position endpos1(static_cast<plugin::File>(1 + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (1 + static_cast<int>(position.rank_get())));
        QuietMove move1(color_piece, position, endpos1, piece_type.value(), piecetype_get(endpos1)!= std::experimental::nullopt , false);
        if(RuleChecker::isMoveAuthorized(*this, move1) && RuleChecker::isMoveLegal(*this, move1))
          moves.push_back(move1);

        plugin::Position endpos3(static_cast<plugin::File>(1 + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (1 + static_cast<int>(position.rank_get())));
        QuietMove move3(color_piece, position, endpos3, piece_type.value(), piecetype_get(endpos3)!= std::experimental::nullopt , false);
        if(RuleChecker::isMoveAuthorized(*this, move3) && RuleChecker::isMoveLegal(*this, move3))
          moves.push_back(move3);

        plugin::Position endpos2(static_cast<plugin::File>(0 + static_cast<int>(position.file_get())), static_cast<plugin::Rank> (1 + static_cast<int>(position.rank_get())));
        QuietMove move2(color_piece, position, endpos2, piece_type.value(), piecetype_get(endpos2)!= std::experimental::nullopt , false);
        if(RuleChecker::isMoveAuthorized(*this, move2) && RuleChecker::isMoveLegal(*this, move2))
          moves.push_back(move2);
      }
      break;

    default:
      throw "Impossible";
      break;
  }

  return moves;
}

std::vector<Piece*> ChessBoard::get_piece(plugin::Color color)
{
  std::vector<Piece*> pieces;
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8 ; j++)
    {
      plugin::Position position(static_cast<plugin::File>(i), static_cast<plugin::Rank>(j));
      if(color_get(position) == color)
      {
        std::experimental::optional<plugin::PieceType> piece_type = piecetype_get(position);
        if (piece_type == std::experimental::nullopt)
          continue;
        switch (piece_type.value())
        {
          case plugin::PieceType::KING:
            pieces.push_back(new King(color_get(position), position, has_moved(position)));
            break;

          case plugin::PieceType::QUEEN:
            pieces.push_back(new Queen(color_get(position), position, has_moved(position)));
            break;

          case plugin::PieceType::ROOK:
            pieces.push_back(new Rook(color_get(position), position, has_moved(position)));
            break;

          case plugin::PieceType::BISHOP:
            pieces.push_back(new Bishop(color_get(position), position, has_moved(position)));
            break;

          case plugin::PieceType::KNIGHT:
            pieces.push_back(new Knight(color_get(position), position, has_moved(position)));
            break;

          case plugin::PieceType::PAWN:
            pieces.push_back(new Pawn(color_get(position), position, has_moved(position)));
            break;

          default:
            exit(10);
            break;
        }
      }
    }
  return pieces;
}

//King(Color color, Position pos, bool has_moved,PieceType piece);

History ChessBoard::history_get() const {
  return history_;
}

plugin::Position ChessBoard::initial_king_position(plugin::Color c)
{
  if (c == plugin::Color::WHITE)
    return plugin::Position(plugin::File::E, plugin::Rank::ONE);
  return plugin::Position(plugin::File::E, plugin::Rank::EIGHT);
}

plugin::Position ChessBoard::initial_rook_position(plugin::Color c, bool king_side)
{
  return plugin::Position(king_side ? plugin::File::H : plugin::File::A, c == plugin::Color::WHITE ? plugin::Rank::ONE : plugin::Rank::EIGHT);
}

plugin::Position ChessBoard::castling_king_end_position(plugin::Color color, bool king_side)
{
  return plugin::Position(king_side ? plugin::File::G : plugin::File::C, color == plugin::Color::WHITE ? plugin::Rank::ONE : plugin::Rank::EIGHT);
}

plugin::Position ChessBoard::castling_rook_end_position(plugin::Color color, bool king_side)
{
  return plugin::Position(king_side ? plugin::File::F : plugin::File::D, color == plugin::Color::WHITE ? plugin::Rank::ONE : plugin::Rank::EIGHT);
}

plugin::Position ChessBoard::get_king_position(plugin::Color color)
{
  for (auto i = 0; i < 8; i++)
  {
    for (auto j = 0; j < 8; j++)
    {
      auto p = plugin::Position(static_cast<plugin::File>(j),
               static_cast<plugin::Rank>(i));
      if (piecetype_get(p) == plugin::PieceType::KING and color_get(p) ==
          color)
        return p;
    }
  }
}
