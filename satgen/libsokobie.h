/*
 * libsokobie.h - libsokobie main header
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __LIBSOKOBIE_LIBSOKOBIE_H_
#define __LIBSOKOBIE_LIBSOKOBIE_H_

#include <string>
#include <istream>
#include <glibmm.h>

namespace Sokobie
{

enum {
  BLOCK_EMPTY = 0,
  BLOCK_WALL = 1,
  BLOCK_BOX = 2,
  BLOCK_PLAYER = 3,
  BLOCK_ENTITY_MASK = 3,
  BLOCK_TARGET = 4, /* target for box */
};

enum Move
{
  MOVE_LEFT = 0,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  PUSH_LEFT,
  PUSH_RIGHT,
  PUSH_UP,
  PUSH_DOWN,
};

typedef std::vector<Move> Moves;
typedef Moves::iterator MovesIter;
typedef Moves::const_iterator MovesConstIter;

class Level
{
private:
  std::string name;
  guint32 columns_n;
  guint32 rows_n;
  guchar* blocks;

  void partial_fill(guchar* mask, guint32 x, guint32 y, Move move) const;
public:
  Level();
  Level(const std::string& name, guint32 columns_n, guint32 rows_n);
  ~Level();

  Level(const Level& level);
  Level& operator=(const Level& level);

  guint32 get_rows_n() const
  { return rows_n; }
  guint32 get_columns_n ()const
  { return columns_n; }
  const guchar* get_blocks() const
  { return blocks; }
  guchar* get_blocks()
  { return blocks; }

  const std::string get_name() const
  { return name; }

  void set_name(const std::string& name)
  { this->name = name; }
  void clear();
  void resize(guint32 columns_n, guint32 rows_n);
  guchar* fill_mask() const;

  bool check_consistency() const;
};

/*
 * SokobieError
 */

class SokobieError: public Glib::Error
{
public:
  enum Code
  {
    BAD_FORMAT,
    OPEN_ERROR,
    IO_ERROR,
    END_OF_FILE,
  };

  SokobieError(); /* default is syntax error */
  SokobieError(int code, const Glib::ustring& text);
  Code code() const;
  Glib::ustring what() const;
};

/*
 * LevelSet - container of levels
 */

class LevelSet
{
private:
  std::string name;
  std::vector<Level> levels;
public:
  LevelSet();
  ~LevelSet();

  const std::string& get_name() const
  { return name; }
  void set_name(const std::string& name)
  { this->name = name; }

  guint32 get_levels_n() const
  { return levels.size(); }
  const Level& get_level(guint number) const
  { return levels[number]; }
  Level& get_level(guint number)
  { return levels[number]; }

  void load_from_file(const std::string& filename);
  void load_from_istream(std::istream& istream);
  void save_to_file(const std::string& filename) const;

  bool check_levels_consistency() const;
};

/*
 * MovesSet - moves set class (moves for all levels)
 */

class MovesSet
{
private:
  std::vector<Moves> moves_set;
public:
  MovesSet();
  ~MovesSet();

  guint get_size() const
  { return moves_set.size (); }
  void set_size(guint size)
  { moves_set.resize (size); }

  const Moves& get_moves(guint32 number) const
  { return moves_set[number]; }
  Moves& get_moves(guint32 number)
  { return moves_set[number]; }

  void set_moves(guint32 number, const Moves& moves)
  { moves_set[number] = moves; }

  void clear()
  { moves_set.clear (); }

  void load_from_file(const std::string& filename);
  void save_to_file(const std::string& filename) const;
};

/*
 * GameState - game state.
 */

class GameState
{
private:
  Level current;
  Moves moves;
  guint pushes_n;
  guint32 player_x, player_y;
  guint32 boxes_n;
  guint32 boxes_in_target;

public:
  GameState();
  explicit GameState(const Level& level);
  ~GameState();

  bool do_move(Move move); /* return true if good move */
  bool check_move(Move move) const;
  void undo_move();
  void undo_all();

  void load_moves_from_file(const std::string& filename);
  void save_moves_to_file(const std::string& filename) const;

  const guint get_moves_n() const
  { return moves.size(); }
  const guint get_pushes_n() const
  { return pushes_n; }
  const Moves& get_moves() const
  { return moves; }
  Moves& get_moves()
  { return moves; }
  void set_moves(const Moves& inmoves);

  guint32 get_boxes_n() const
  { return boxes_n; }
  guint32 get_boxes_in_target_n() const
  { return boxes_in_target; }
  guint32 get_player_x() const
  { return player_x; }
  guint32 get_player_y() const
  { return player_y; }

  void change_level(const Level& level);

  const Level& get_current() const
  { return current; }
  Level& get_current()
  { return current; }

  bool is_solution() const
  { return boxes_n == boxes_in_target; }
};

};


#endif /* __LIBSOKOBIE_LIBSOKOBIE_H_ */
