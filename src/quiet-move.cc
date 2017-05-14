#include "quiet-move.hh"

QuietMove::QuietMove(plugin::Color color, plugin::Position start,
                     plugin::Position end, plugin::PieceType type, bool attack,
                     bool promotion)
  : Move(QUIET, color)
  , start_(start)
  , end_(end)
  , piecetype_(type)
  , attack_(attack)
  , promotion_(promotion)
{
  std::cerr << "Creating QuietMove" << std::endl;
}

plugin::PieceType QuietMove::piecetype_get() const
{
  return piecetype_;
}

plugin::Position QuietMove::start_get() const
{
  return start_;
}

plugin::Position QuietMove::end_get() const
{
  return end_;
}

bool QuietMove::is_an_attack() const
{
  return attack_;
}

bool QuietMove::is_promotion() const
{
  return promotion_;
}

bool QuietMove::is_a_test() const
{
  return test_;
}

std::ostream& operator<<(std::ostream& o, const plugin::Position p)
{
  o << (char)(static_cast<char>(p.file_get()) + 'A')
    << (char)(static_cast<char>(p.rank_get()) + '1');
  return o;
}

void QuietMove::print(std::ostream& o) const
{
  o << static_cast<char>(piecetype_get()) << (attack_ ? " attack" : " move")
    << " from " << start_get() << " to " << end_get();
}
