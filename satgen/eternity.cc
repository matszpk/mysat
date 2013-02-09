/*
 * eternity.cc
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <sstream>
#include "gen-arith.h"
#include "eternity.h"

EternityModule::EternityModule()
{
}


EternityModule::~EternityModule()
{
}

SatGenModule* EternityModule::create()
{
  return static_cast<SatGenModule*>(new EternityModule);
}

void EternityModule::post_process_params()
{
}


void EternityModule::parse_input(const std::string& input)
{
  board_data.clear();
  std::istringstream is(input);

  while (!is.eof())
  {
    gint64 value;
    is >> value;
    if (is.eof())
      break;
    if (is.fail())
      throw InputSyntaxError ("Bad number");
    board_data.push_back (value);
  }

  guint square_size = 0;
  for (square_size = 0; square_size*square_size*4 < board_data.size(); square_size++);
  if (square_size*square_size*4 != board_data.size())
    throw InputSyntaxError ("Bad size");
}

void EternityModule::generate(CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  guchar quarts_map[256];
  guchar reversed_quarts_map[256];

  guint piece_bits = 0;
  guint pieces_num = board_data.size()/4;
  for (piece_bits = 0; (1ULL<<piece_bits) < pieces_num; piece_bits++);

  std::fill(quarts_map, quarts_map+256, guchar(255));

  guint quart_bits = 0;
  guint quarts_num = 0;
  std::vector<guchar> sorted(board_data);
  std::sort(sorted.begin(), sorted.end());

  for (guint i=0; i < sorted.size(); i++)
    if (quarts_map[sorted[i]] == 255)
    {
      reversed_quarts_map[quarts_num] = sorted[i];
      quarts_map[sorted[i]] = quarts_num++;
    }

  for (quart_bits = 0; (1ULL<<quart_bits) < quarts_num; quart_bits++);

  gint32 start_pieces = cnf.next_var();
  cnf.add_vars((piece_bits+2)*pieces_num);

  // generate constraint: piece[i]!=pieces[j] for all pairs
  for (guint i=1; i < pieces_num; i++)
    for (guint j=0; j < i; j++)
    {
      LiteralVector a = get_literals_from_range (start_pieces + i*(piece_bits+2)+2,
	      start_pieces + (i+1)*(piece_bits+2));
      LiteralVector b = get_literals_from_range (start_pieces + j*(piece_bits+2)+2,
	      start_pieces + (j+1)*(piece_bits+2));
      cnf_gen_equal(cnf, 0, a, b, DEF_NEGATIVE); // a!=b
    }

  gint32 start_quarts = cnf.next_var();
  cnf.add_vars(pieces_num*4*quart_bits);

  // main problema
  guint64* values_map = new guint64[pieces_num*4];

  guint square_size = 0;
  for (square_size = 0; square_size*square_size*4 < board_data.size(); square_size++);

  outmap_string.clear();

  for (guint i=0; i < pieces_num*4; i++)
  {
    guint piece_idx = i>>2;

    LiteralVector quart = get_literals_from_range (start_quarts + i*(quart_bits),
	start_quarts + (i+1)*(quart_bits));

    if (with_outmap)
    {
      std::ostringstream os;
      os << "{ {";
      for (guint v = 0; v < quarts_num; v++)
      {
	os << guint(reversed_quarts_map[v]);
	if (v+1 < quarts_num)
	  os << "|";
      }
      os << "} : ";
      for (guint b = 0; b < quart_bits; b++)
	os << start_quarts + i*(quart_bits) + b << " ";
      os << "}";

      if ((i % (square_size*4)) == square_size*4-1)
	os << "\n";
      else if ((i & 3) == 3)
	os << "  ";
      else
	os << " ";

      outmap_string += os.str();
    }

    LiteralVector index = get_literals_from_range (start_pieces +
	piece_idx*(piece_bits+2), start_pieces + (piece_idx+1)*(piece_bits+2));

    for (guint j=0; j < pieces_num*4; j++)
      values_map[j] = quarts_map[board_data[(j&~3) + ((i+j)&3)]];

    cnf_gen_map(cnf, quart, pieces_num*4, values_map, index);
  }
  delete[] values_map;

  // main constraints
  for (guint y = 0; y <  square_size; y++)
    for (guint x = 0; x <  square_size; x++)
    {
      guint i = x + y*square_size;
      LiteralVector quart_up = get_literals_from_range(start_quarts +
	  (i*4)*(quart_bits), start_quarts + (i*4+1)*(quart_bits));
      LiteralVector quart_right = get_literals_from_range(start_quarts +
	  (i*4+1)*(quart_bits), start_quarts + (i*4+2)*(quart_bits));
      LiteralVector quart_down = get_literals_from_range(start_quarts +
	  (i*4+2)*(quart_bits), start_quarts + (i*4+3)*(quart_bits));
      LiteralVector quart_left = get_literals_from_range(start_quarts +
	  (i*4+3)*(quart_bits), start_quarts + (i*4+4)*(quart_bits));

      if (x == 0)
        cnf_gen_equal(cnf, 0, quart_left, 0, DEF_POSITIVE);
      else
      {
	LiteralVector adjacent = get_literals_from_range(start_quarts +
	    (i*4-3)*(quart_bits), start_quarts + (i*4-2)*(quart_bits));
	cnf_gen_equal(cnf, 0, quart_left, adjacent, DEF_POSITIVE);
      }

      if (x == square_size-1)
	cnf_gen_equal(cnf, 0, quart_right, 0, DEF_POSITIVE);
      else
      {
	LiteralVector adjacent = get_literals_from_range(start_quarts +
	    (i*4+7)*(quart_bits), start_quarts + (i*4+8)*(quart_bits));
	cnf_gen_equal(cnf, 0, quart_right, adjacent, DEF_POSITIVE);
      }

      if (y == 0)
        cnf_gen_equal(cnf, 0, quart_up, 0, DEF_POSITIVE);
      else
      {
	LiteralVector adjacent = get_literals_from_range(start_quarts +
	    ((i-square_size)*4+2)*(quart_bits), start_quarts +
	    ((i-square_size)*4+3)*(quart_bits));
	cnf_gen_equal(cnf, 0, quart_up, adjacent, DEF_POSITIVE);
      }

      if (y == square_size-1)
	cnf_gen_equal(cnf, 0, quart_down, 0, DEF_POSITIVE);
      else
      {
	LiteralVector adjacent = get_literals_from_range(start_quarts +
	    ((i+square_size)*4)*(quart_bits), start_quarts +
	    ((i+square_size)*4+1)*(quart_bits));
	cnf_gen_equal(cnf, 0, quart_down, adjacent, DEF_POSITIVE);
      }
    }
}
