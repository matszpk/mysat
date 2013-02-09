/*
 * libsokobie.cc - main module of libsokobie
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <stack>
#include <cctype>
#include "libsokobie.h"

using namespace Sokobie;

/* Level class implementation */

Level::Level() : name(""), columns_n(0), rows_n(0), blocks(0)
{
}

Level::Level(const std::string& name, guint32 columns_n, guint32 rows_n)
{
  this->name = name;
  this->columns_n = columns_n;
  this->rows_n = rows_n;
  blocks = new guchar[rows_n*columns_n];

  std::fill(blocks, blocks + rows_n*columns_n, guchar (BLOCK_EMPTY));
}

Level::~Level()
{
  delete []blocks;
}

Level::Level(const Level& level) : blocks(0)
{
  *this = level;
}

Level& Level::operator=(const Level& level)
{
  delete []blocks;
  blocks = 0;
  columns_n = level.columns_n;
  rows_n = level.rows_n;
  name = level.name;
  blocks = new guchar[rows_n*columns_n];
  if (rows_n*columns_n != 0 && level.blocks != 0)
    std::copy(level.blocks, level.blocks + columns_n*rows_n, blocks);
  return *this;
}

void Level::clear()
{
  std::fill(blocks, blocks + rows_n*columns_n, guchar(BLOCK_EMPTY));
}

void Level::resize(guint32 newcolumns_n, guint32 newrows_n)
{
  guchar* newblocks = new guchar[newcolumns_n*newrows_n];
  if (blocks != 0 && rows_n*columns_n != 0)
  {
    guint32 mincols_n = std::min(newcolumns_n, columns_n);
    guint32 minrows_n = std::min(newrows_n, rows_n);
    for (guint32 i  = 0; i < minrows_n; i++)
    {
      std::copy(blocks + i*columns_n, blocks + i*columns_n + mincols_n,
	  newblocks + i*newcolumns_n);
      std::fill(newblocks + i*newcolumns_n + mincols_n,
	  newblocks + i*(newcolumns_n+1), guchar(BLOCK_EMPTY));
    }
    for (guint32 i = minrows_n; i < newrows_n; i++)
      std::fill(newblocks + i*newcolumns_n, newblocks + i*(newcolumns_n+1),
	  guchar(BLOCK_EMPTY));
  }
  else
    std::fill(newblocks, newblocks + newrows_n*newcolumns_n, guchar(BLOCK_EMPTY));

  delete []blocks;
  blocks = newblocks;
  columns_n = newcolumns_n;
  rows_n = newrows_n;
}

void Level::partial_fill(guchar* mask, guint32 x, guint32 y, Move move) const
{
  if (mask[y*columns_n+x] != 0 ||
      (blocks[y*columns_n+x] & BLOCK_ENTITY_MASK) == BLOCK_WALL)
    return;

  //mask[y*columns_n+x] = 1;

  switch (move)
  {
    case MOVE_RIGHT:
    {
      guint32 newx = x;
      for (; newx < columns_n; newx++)
	if (mask[y*columns_n+newx] == 0 &&
	    (blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	  mask[y*columns_n+newx] = 1;
	else
	  break;

      guint32 maxx = newx;

      if (y+1 < rows_n)
      {
	newx = x;
	for (; newx < maxx; newx++)
	  if ((blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL &&
	      mask[(y+1)*columns_n+newx] == 0 &&
		(blocks[(y+1)*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	    partial_fill(mask, newx, y+1, MOVE_DOWN);
      }
      if (y != 0)
      {
	newx = x;
	for (; newx < maxx; newx++)
	  if ((blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL &&
	      mask[(y-1)*columns_n+newx] == 0 &&
		(blocks[(y-1)*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	    partial_fill(mask, newx, y-1, MOVE_UP);
      }

      break;
    }
    case MOVE_LEFT:
    {
      gint32 newx = x;
      for (; newx >= 0; newx--)
	if (mask[y*columns_n+newx] == 0 &&
	    (blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	  mask[y*columns_n+newx] = 1;
	else
	  break;

      gint32 minx = newx;

      if (y+1 < rows_n)
      {
	newx = x;
	for (; newx > minx; newx--)
	  if ((blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL &&
	      mask[(y+1)*columns_n+newx] == 0 &&
		(blocks[(y+1)*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	    partial_fill(mask, newx, y+1, MOVE_DOWN);
      }
      if (y != 0)
      {
	newx = x;
	for (; newx > minx; newx--)
	  if ((blocks[y*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL &&
	      mask[(y-1)*columns_n+newx] == 0 &&
		(blocks[(y-1)*columns_n+newx] & BLOCK_ENTITY_MASK) != BLOCK_WALL)
	    partial_fill(mask, newx, y-1, MOVE_UP);
      }

      break;
    }
    case MOVE_UP:
    case MOVE_DOWN:
    {
      //mask[y*columns_n+x] = 1;
      guint32 newy = y;
      if (newy < rows_n)
      {
	//if (x < rows_n-1)
	partial_fill(mask, x, newy, MOVE_RIGHT);
        if (x != 0)
          partial_fill(mask, x-1, newy, MOVE_LEFT);
      }
      else if (y != 0)
	partial_fill (mask, x, y-1, MOVE_UP);
      break;
    }
    default:
      break;
  }
}

guchar* Level::fill_mask() const
{
  guchar* mask = new guchar[rows_n*columns_n];
  std::fill(mask, mask + rows_n*columns_n, 0);
  for (guint32 x = 0; x < columns_n; x++)
  {
    partial_fill(mask, x, 0, MOVE_DOWN);
    partial_fill(mask, x, rows_n-1, MOVE_UP);
  }

  for (guint32 y = 0; y < rows_n; y++)
  {
    partial_fill(mask, 0, y, MOVE_RIGHT);
    partial_fill(mask, columns_n-1, y, MOVE_LEFT);
  }
  return mask;
}

bool Level::check_consistency() const
{
  bool player = false;
  guint32 target_count = 0;
  guint32 box_count = 0;

  bool consistent = true;
  /* get outside level filled space */
  guchar* mask = fill_mask();

  for (guint32 i = 0; i < columns_n*rows_n; i++)
  {
    guchar entity = blocks[i] & BLOCK_ENTITY_MASK;
    /* only one player is permitted, target at wall is not permitted */
    if ((entity == BLOCK_PLAYER && player) ||
	(blocks[i] & BLOCK_TARGET && entity == BLOCK_WALL) ||
	(mask[i] != 0 && (entity != BLOCK_EMPTY || blocks[i] == BLOCK_TARGET)))
    {
      consistent = false;
      break;
    }
    if (entity == BLOCK_PLAYER)
      player = true;
    if (entity == BLOCK_BOX)
      box_count++;
    if (blocks[i] & BLOCK_TARGET)
      target_count++;
  }

  delete []mask;
  return (consistent && player && target_count == box_count);
}

/*
 * SokobieError
 */

SokobieError::SokobieError()
    : Glib::Error (Glib::Quark("LibSokobie"),
	SokobieError::IO_ERROR, "IO Error")
{
}

SokobieError::SokobieError(int code, const Glib::ustring& text)
    : Glib::Error (Glib::Quark("LibSokobie"), code, text)
{
}

SokobieError::Code SokobieError::code() const
{
  return static_cast<SokobieError::Code>(Glib::Error::code());
}

Glib::ustring SokobieError::what() const
{
  Glib::ustring out = "I/O: ";
  out += Glib::Error::what();
  return out;
}

/* LevelSet implementation */

LevelSet::LevelSet()
{
}

LevelSet::~LevelSet()
{
}

namespace
{

std::string get_name_from_comment_line(const std::string& line)
{
  std::string name;
  std::string::const_iterator line_it = line.begin();
  std::string::const_iterator line_end = line.end();
  if (*line_it  == ';')
    ++line_it;
  if (line_it != line_end && *line_it == ' ')
    line_it++;
  if (line.begin() != line_end && line_end[-1] == '\r')
    --line_end;
  if (line_it != line_end)
    name.assign(line_it, line_end);
  else
    name.clear();
  return name;
}

std::string truncate_last_spaces(const std::string& line)
{
  std::string newline;
  std::string::size_type pos = line.find_last_not_of("\r\t\v\f ");
  if (pos != std::string::npos)
    newline = line.substr(0, pos+1);
  else
    newline.clear();
  return newline;
}

}

void LevelSet::load_from_istream(std::istream& file)
{
  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");

  file.exceptions(std::ios::badbit);
  std::vector<Level> new_levels;

  try
  {
    guint32 line_count = 1;
    std::string prev_comment_line = "";
    std::string prev_line = "";
    std::string line;

    bool level_fetch = false;
    guint32 level_rows_n = 0;
    guint32 level_columns_n = 0;
    std::string level_lines;

    bool is_end_of_file = false;

    /* skip comments */
    while (!is_end_of_file)
    {
      std::getline(file, line);
      if (line.size() == 0 && file.eof())
        is_end_of_file = true;

      std::string::const_iterator line_it = line.begin();
      std::string::const_iterator line_end = line.end();

      if (line_count == 1)
      { /* parse first line */
	if (line_it != line_end && *line_it != ';')
	  throw SokobieError(SokobieError::BAD_FORMAT,
	      "First line must be as comment");

	if (line_it != line_end)
	  name = get_name_from_comment_line(line);
	else
	  name.clear();
      }

      bool is_level_line = false;

      if (!level_fetch)
      {
	if (!line.empty())
	{
	  bool is_hash = false;
	  for (line_it = line.begin(); line_it != line_end; ++line_it)
	  {
	    if (*line_it == '#')
	      is_hash = true;
	    if (*line_it != '#' && *line_it != ' ')
	      break;
	  }
	  if ((line_it == line_end ||
	      (line_it+1 == line_end && *line_it == '\r')) && is_hash)
	    is_level_line = true;

	  line_it = line.begin();
	}

	if (is_level_line && prev_comment_line.empty() && prev_line.empty())
	  throw SokobieError(SokobieError::BAD_FORMAT, "Level must be name");

	if (is_level_line)
	{
	  std::string level_line = truncate_last_spaces(line);
	  new_levels.push_back (Level());
	  if (!prev_comment_line.empty())
	  {
	    new_levels.back().set_name(
		get_name_from_comment_line(prev_comment_line));
	    prev_comment_line.clear();
	    if (new_levels.back().get_name().empty())
	    {
	      new_levels.back().set_name(
		get_name_from_comment_line(prev_line));
	      prev_line.clear();
	    }
	  }
	  else
	  {
	    new_levels.back().set_name(get_name_from_comment_line(prev_line));
	    prev_line.clear();
	  }
	  level_columns_n = level_line.size();
	  level_rows_n = 1;
	  level_lines = level_line;
	  level_lines += '\n';
	  level_fetch = true;
	}
      }
      else /* fetching next line of level */
      {
	bool other_than_space = false;
	for (line_it = line.begin(); line_it != line_end; ++line_it)
	{
	    if (*line_it != '#' && *line_it != ' ' && *line_it != '.' &&
		*line_it != '$' && *line_it != '*' && *line_it != '@' &&
		*line_it != '+')
	      break;
	    if (*line_it != ' ')
	      other_than_space = true;
	}
	if ((line_it == line_end ||
	      (line_it+1 == line_end && *line_it == '\r')) && other_than_space)
	{
	  std::string level_line = truncate_last_spaces (line);
	  is_level_line = true;
	  level_columns_n = std::max(level_columns_n, guint32(level_line.size()));
	  level_rows_n++;
	  level_lines += level_line;
	  level_lines += '\n';
	}
	else
	  is_level_line = false;

	line_it = line.begin();
      }

      /* if level fetch mode and if not level line (then level end) */
      if (level_fetch && !is_level_line)
      {
	level_fetch = false;
	/* now put level string to level instance */
	Level& level = new_levels.back();
	level.resize(level_columns_n, level_rows_n);

	std::string::const_iterator it = level_lines.begin();
	std::string::const_iterator end = level_lines.end();

	guint32 x = 0;
	guint32 y = 0;
	guchar* blocks = level.get_blocks();
	for (; it != end; ++it)
	{
	  guchar block = BLOCK_EMPTY;
	  if (*it != '\n')
	  {
	    switch (*it)
	    {
	      case ' ':
		block = BLOCK_EMPTY;
		break;
	      case '#':
		block = BLOCK_WALL;
		break;
	      case '.':
		block = BLOCK_TARGET;
		break;
	      case '$':
		block = BLOCK_BOX;
		break;
	      case '*':
		block = BLOCK_BOX | BLOCK_TARGET;
		break;
	      case '@':
		block = BLOCK_PLAYER;
		break;
	      case '+':
		block = BLOCK_PLAYER | BLOCK_TARGET;
		break;
	      default:
		break;
	    }
	    blocks[y*level_columns_n+x] = block;
	    x++;
	  }
	  else /* end of row */
	  {
	    for (; x < level_columns_n; x++)
	      blocks[y*level_columns_n+x] = BLOCK_EMPTY;
	    x = 0;
	    y++;
	  }
	}
	level_lines.clear();
      }

      /* copy to previous comment line */
      if (line_it != line_end && *line_it == ';' && line_count > 1)
	prev_comment_line = line;
      if (!is_level_line && (line_it != line_end &&
	   !(line_it+1 == line_end && *line_it == '\r'))  && line_count > 1)
	prev_line = line;
      line_count++;
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }
  levels = new_levels;
}

void LevelSet::load_from_file(const std::string& filename)
{
  std::ifstream file(filename.c_str());

  load_from_istream(file);
}

void LevelSet::save_to_file(const std::string& filename) const
{
  std::ofstream file(filename.c_str ());

  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");

  file.exceptions(std::ios::badbit);

  try
  {
    file << "; " << name << "\n; Saved by Sokobie LevelSetIO\n\n";

    for (guint i = 0; i < levels.size(); i++)
    {
      guint32 rows_n = levels[i].get_rows_n();
      guint32 columns_n = levels[i].get_columns_n();
      const guchar* blocks = levels[i].get_blocks();

      file << "; " << levels[i].get_name() << '\n';

      for (guint32 y = 0; y < rows_n; y++)
      {
	for (guint32 x = 0; x < columns_n; x++)
	{
	  char c;
	  switch (blocks[y*columns_n+x])
	  {
	    case BLOCK_EMPTY:
	      c = ' ';
	      break;
	    case BLOCK_TARGET:
	      c = '.';
	      break;
	    case BLOCK_WALL:
	      c = '#';
	      break;
	    case BLOCK_BOX:
	      c = '$';
	      break;
	    case BLOCK_BOX | BLOCK_TARGET:
	      c = '*';
	      break;
	    case BLOCK_PLAYER:
	      c = '@';
	      break;
	    case BLOCK_PLAYER | BLOCK_TARGET:
	      c = '+';
	      break;
	    default:
	      c = ' ';
	      break;
	  }
	  file << c;
	}
	file << '\n';
      }
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }
}

bool LevelSet::check_levels_consistency() const
{
  for (guint i = 0; i < levels.size(); i++)
    if (!levels[i].check_consistency())
      return false;
  return true;
}

/*
 * MovesSet - moves set class (moves for all levels)
 */

MovesSet::MovesSet()
{
}

MovesSet::~MovesSet()
{
}

void MovesSet::load_from_file(const std::string& filename)
{
  std::ifstream file(filename.c_str());
  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");
  file.exceptions(std::ios::badbit);

  std::vector<Moves> new_moves_set;
  Moves new_moves;

  try
  {
    while (true)
    {
      char c;
      file >> c;
      if (file.eof())
	break;

      Move move;
      bool no_move = false;
      switch (tolower(c))
      {
	case 'l':
	  move = MOVE_LEFT;
	  break;
	case 'r':
	  move = MOVE_RIGHT;
	  break;
	case 'u':
	  move = MOVE_UP;
	  break;
	case 'd':
	  move = MOVE_DOWN;
	  break;
	case ';': /* end of moves */
	  new_moves_set.push_back(new_moves);
	  new_moves.clear();
	  no_move = true;
	  break;
	default:
	  throw SokobieError(SokobieError::BAD_FORMAT, "Bad format of moves set");
	  break;
      }
      if (!no_move)
	new_moves.push_back(move);
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }

  if (!new_moves.empty())
    new_moves_set.push_back(new_moves);
  moves_set = new_moves_set;
}

void MovesSet::save_to_file(const std::string& filename) const
{
  std::ofstream file(filename.c_str());
  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");
  file.exceptions(std::ios::badbit);

  try
  {
    for (std::vector<Moves>::const_iterator moves = moves_set.begin();
	 moves != moves_set.end(); ++moves)
    {
      for (MovesConstIter it = moves->begin(); it != moves->end(); ++it)
      {
	char c = 'x';
	switch (*it)
	{
	  case MOVE_LEFT:
	  case PUSH_LEFT:
	    c = 'l';
	    break;
	  case MOVE_RIGHT:
	  case PUSH_RIGHT:
	    c = 'r';
	    break;
	  case MOVE_UP:
	  case PUSH_UP:
	    c = 'u';
	    break;
	  case MOVE_DOWN:
	  case PUSH_DOWN:
	    c = 'd';
	    break;
	  default:
	    break;
	}
	file << c;
      }
      file << ";\n";
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }
}

/*
 * GameState - game state implementation
 */

GameState::GameState()
{
}

GameState::GameState(const Level& level)
{
  change_level(level);
}

GameState::~GameState()
{
}


bool GameState::do_move(Move move)
{
  bool is_moved = false;
  Move dest_move;
  guint32 cols_n = current.get_columns_n();
  guint32 rows_n = current.get_rows_n();
  guchar* blocks = current.get_blocks();
  bool is_pushed = false;
  guchar block = BLOCK_EMPTY; /* block where next position of player */
  guchar block2 = BLOCK_EMPTY; /* block where next position of pushed box */
  guint32 oldpos = cols_n*player_y+player_x;

  switch (move)
  {
    case MOVE_LEFT:
    case PUSH_LEFT:
      if (player_x > 0)
      {
	block = blocks[player_y*cols_n+player_x-1];
	if (block == BLOCK_EMPTY || block == BLOCK_TARGET)
	{
	  dest_move = MOVE_LEFT;
	  is_moved = true;
	}
	else if ((block & BLOCK_ENTITY_MASK) == BLOCK_BOX && player_x > 1)
	{
	  block2 = blocks[player_y*cols_n+player_x-2];
	  if (block2 == BLOCK_EMPTY || block2 == BLOCK_TARGET)
	  {
	    dest_move = PUSH_LEFT;
	    blocks[player_y*cols_n+player_x-2] =
	      (blocks[player_y*cols_n+player_x-2] & BLOCK_TARGET) | BLOCK_BOX;
	    is_pushed = is_moved = true;
	  }
	}
	if (is_moved)
	{
	  blocks[player_y*cols_n+player_x-1] =
	    (blocks[player_y*cols_n+player_x-1] & BLOCK_TARGET) | BLOCK_PLAYER;
	  player_x--;
	}
      }
      break;
    case MOVE_RIGHT:
    case PUSH_RIGHT:
      if (player_x < cols_n-1)
      {
	block = blocks[player_y*cols_n+player_x+1];
	if (block == BLOCK_EMPTY || block == BLOCK_TARGET)
	{
	  dest_move = MOVE_RIGHT;
	  is_moved = true;
	}
	else if ((block & BLOCK_ENTITY_MASK) == BLOCK_BOX && player_x < cols_n-2)
	{
	  block2 = blocks[player_y*cols_n+player_x+2];
	  if (block2 == BLOCK_EMPTY || block2 == BLOCK_TARGET)
	  {
	    dest_move = PUSH_RIGHT;
	    blocks[player_y*cols_n+player_x+2] =
	      (blocks[player_y*cols_n+player_x+2] & BLOCK_TARGET) | BLOCK_BOX;
	    is_pushed = is_moved = true;
	  }
	}
	if (is_moved)
	{
	  blocks[player_y*cols_n+player_x+1] =
	    (blocks[player_y*cols_n+player_x+1] & BLOCK_TARGET) | BLOCK_PLAYER;
	  player_x++;
	}
      }
      break;
    case MOVE_UP:
    case PUSH_UP:
      if (player_y > 0)
      {
	block = blocks[(player_y-1)*cols_n+player_x];
	if (block == BLOCK_EMPTY || block == BLOCK_TARGET)
	{
	  dest_move = MOVE_UP;
	  is_moved = true;
	}
	else if ((block & BLOCK_ENTITY_MASK) == BLOCK_BOX && player_y > 1)
	{
	  block2 = blocks[(player_y-2)*cols_n+player_x];
	  if (block2 == BLOCK_EMPTY || block2 == BLOCK_TARGET)
	  {
	    dest_move = PUSH_UP;
	    blocks[(player_y-2)*cols_n+player_x] =
	      (blocks[(player_y-2)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	    is_pushed = is_moved = true;
	  }
	}
	if (is_moved)
	{
	  blocks[(player_y-1)*cols_n+player_x] =
	    (blocks[(player_y-1)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_PLAYER;
	  player_y--;
	}
      }
      break;
    case MOVE_DOWN:
    case PUSH_DOWN:
      if (player_y < rows_n-1)
      {
	block = blocks[(player_y+1)*cols_n+player_x];
	if (block == BLOCK_EMPTY || block == BLOCK_TARGET)
	{
	  dest_move = MOVE_DOWN;
	  is_moved = true;
	}
	else if ((block & BLOCK_ENTITY_MASK) == BLOCK_BOX && player_y < rows_n-2)
	{
	  block2 = blocks[(player_y+2)*cols_n+player_x];
	  if (block2 == BLOCK_EMPTY || block2 == BLOCK_TARGET)
	  {
	    dest_move = PUSH_DOWN;
	    blocks[(player_y+2)*cols_n+player_x] =
	      (blocks[(player_y+2)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	    is_pushed = is_moved = true;
	  }
	}
	if (is_moved)
	{
	  blocks[(player_y+1)*cols_n+player_x] =
	    (blocks[(player_y+1)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_PLAYER;
	  player_y++;
	}
      }
      break;
    default:
      break;
  }

  if (is_pushed)
  {
    if (block & BLOCK_TARGET)
      boxes_in_target--;
    if (block2 & BLOCK_TARGET)
      boxes_in_target++;
    pushes_n++;
  }

  if (is_moved)
  { /* free previous position */
    blocks[oldpos] &= BLOCK_TARGET;
    moves.push_back(dest_move);
  }
  return is_moved;
}

void GameState::undo_move ()
{
  if (moves.empty())
    return;

  bool is_pushed = false;
  guint32 cols_n = current.get_columns_n();
  guchar* blocks = current.get_blocks();
  guchar block = BLOCK_EMPTY; /* block where current position of player */
  guchar block2 = BLOCK_EMPTY; /* block where current position of pushed box */

  Move prev_move = moves.back();

  switch (prev_move)
  {
    case MOVE_LEFT:
    case PUSH_LEFT:
      block = blocks[player_y*cols_n+player_x];
      blocks[player_y*cols_n+player_x+1] =
	(blocks[player_y*cols_n+player_x+1] & BLOCK_TARGET) | BLOCK_PLAYER;
      if (prev_move == PUSH_LEFT)
      {
	block2 = blocks[player_y*cols_n+player_x-1];
	blocks[player_y*cols_n+player_x] =
	  (blocks[player_y*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	blocks[player_y*cols_n+player_x-1] &= BLOCK_TARGET;
	is_pushed = true;
      }
      else
	blocks[player_y*cols_n+player_x] &= BLOCK_TARGET;
      player_x++;
      break;
    case MOVE_RIGHT:
    case PUSH_RIGHT:
      block = blocks[player_y*cols_n+player_x];
      blocks[player_y*cols_n+player_x-1] =
	(blocks[player_y*cols_n+player_x-1] & BLOCK_TARGET) | BLOCK_PLAYER;
      if (prev_move == PUSH_RIGHT)
      {
	block2 = blocks[player_y*cols_n+player_x+1];
	blocks[player_y*cols_n+player_x] =
	  (blocks[player_y*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	blocks[player_y*cols_n+player_x+1] &= BLOCK_TARGET;
	is_pushed = true;
      }
      else
	blocks[player_y*cols_n+player_x] &= BLOCK_TARGET;
      player_x--;
      break;
    case MOVE_UP:
    case PUSH_UP:
      block = blocks[player_y*cols_n+player_x];
      blocks[(player_y+1)*cols_n+player_x] =
	(blocks[(player_y+1)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_PLAYER;
      if (prev_move == PUSH_UP)
      {
	block2 = blocks[(player_y-1)*cols_n+player_x];
	blocks[player_y*cols_n+player_x] =
	  (blocks[player_y*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	blocks[(player_y-1)*cols_n+player_x] &= BLOCK_TARGET;
	is_pushed = true;
      }
      else
	blocks[player_y*cols_n+player_x] &= BLOCK_TARGET;
      player_y++;
      break;
    case MOVE_DOWN:
    case PUSH_DOWN:
      block = blocks[player_y*cols_n+player_x];
      blocks[(player_y-1)*cols_n+player_x] =
	(blocks[(player_y-1)*cols_n+player_x] & BLOCK_TARGET) | BLOCK_PLAYER;
      if (prev_move == PUSH_DOWN)
      {
	block2 = blocks[(player_y+1)*cols_n+player_x];
	blocks[player_y*cols_n+player_x] =
	  (blocks[player_y*cols_n+player_x] & BLOCK_TARGET) | BLOCK_BOX;
	blocks[(player_y+1)*cols_n+player_x] &= BLOCK_TARGET;
	is_pushed = true;
      }
      else
	blocks[player_y*cols_n+player_x] &= BLOCK_TARGET;
      player_y--;
      break;
    default:
      break;
  }

  if (is_pushed)
  {
    if (block & BLOCK_TARGET)
      boxes_in_target++;
    if (block2 & BLOCK_TARGET)
      boxes_in_target--;
    pushes_n--;
  }

  moves.pop_back();
}


void GameState::undo_all ()
{
  while (!moves.empty())
    undo_move();
}

void
GameState::load_moves_from_file(const std::string& filename)
{
  std::ifstream file(filename.c_str());
  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");
  file.exceptions(std::ios::badbit);

  Moves new_moves;

  try
  {
    while (true)
    {
      char c;
      file >> c;
      if (file.eof())
	break;

      Move move;
      switch (tolower(c))
      {
	case 'l':
	  move = MOVE_LEFT;
	  break;
	case 'r':
	  move = MOVE_RIGHT;
	  break;
	case 'u':
	  move = MOVE_UP;
	  break;
	case 'd':
	  move = MOVE_DOWN;
	  break;
	default:
	  throw SokobieError(SokobieError::BAD_FORMAT, "Bad format of moves");
	  break;
      }
      new_moves.push_back(move);
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }

  undo_all();
  for (MovesConstIter it = new_moves.begin(); it != new_moves.end(); ++it)
    do_move(*it);
}

void GameState::save_moves_to_file(const std::string& filename) const
{
  std::ofstream file(filename.c_str());
  if (!file)
    throw SokobieError(SokobieError::OPEN_ERROR, "Open error");
  file.exceptions(std::ios::badbit);

  try
  {
    for (MovesConstIter it = moves.begin(); it != moves.end(); ++it)
    {
      char c = 'x';
      switch (*it)
      {
	case MOVE_LEFT:
	case PUSH_LEFT:
	  c = 'l';
	  break;
	case MOVE_RIGHT:
	case PUSH_RIGHT:
	  c = 'r';
	  break;
	case MOVE_UP:
	case PUSH_UP:
	  c = 'u';
	  break;
	case MOVE_DOWN:
	case PUSH_DOWN:
	  c = 'd';
	  break;
	default:
	  break;
      }
      file << c;
    }
  }
  catch (std::ios::failure& ex)
  {
    throw SokobieError(SokobieError::IO_ERROR, "I/O error");
  }
}

void GameState::set_moves(const Moves& inmoves)
{
  undo_all();
  for (MovesConstIter it = inmoves.begin(); it != inmoves.end(); ++it)
    do_move(*it);
}

void GameState::change_level(const Level& level)
{
  moves.clear();
  pushes_n = 0;
  current = level;
  boxes_n = boxes_in_target = 0;

  guint32 cols_n = current.get_columns_n();
  guint32 rows_n = current.get_rows_n();
  const guchar* blocks = current.get_blocks();

  for (guint y = 0; y < rows_n; y++)
    for (guint x = 0; x < cols_n; x++)
    {
      guchar block = blocks[y*cols_n+x];
      if (block == BLOCK_BOX)
	boxes_n++;
      else if (block == (BLOCK_BOX | BLOCK_TARGET))
      {
	boxes_n++;
	boxes_in_target++;
      }
      if ((block & BLOCK_ENTITY_MASK) == BLOCK_PLAYER)
      {
	player_x = x;
	player_y = y;
      }
    }
}
