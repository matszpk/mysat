/*
 * sokoban.cc
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <sstream>
#include "gen-arith.h"
#include "sokoban.h"

using namespace SatUtils;
using namespace Sokobie;

SokobanModule::SokobanModule(): moves_n(100)
{
  add_param ("moves", "Maximum number of moves", moves_n);
  add_param ("levelno", "Level number", levelno);
}


SokobanModule::~SokobanModule()
{
}

SatGenModule* SokobanModule::create()
{
  return static_cast<SatGenModule*>(new SokobanModule);
}

void SokobanModule::post_process_params()
{
  if (moves_n == 0)
    throw ModuleError(ModuleError::BAD_VALUE,
          "Number of moves must be greater than 0.");
  if (moves_n == 0)
    throw ModuleError(ModuleError::BAD_VALUE,
          "Level number must be grater than 0.");
}


void SokobanModule::parse_input(const std::string& input)
{
  std::istringstream iss(input);
  LevelSet levelset;
  try
  { levelset.load_from_istream(iss); }
  catch(SokobieError& error)
  { throw InputSyntaxError ("Cant parse levelset"); }

  if (levelset.get_levels_n() <= levelno)
    throw InputSyntaxError ("Bad number levelset");

  level = levelset.get_level(levelno);
}

void SokobanModule::generate(CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  guchar* fill_mask = level.fill_mask();

  LiteralVector solveds = cnf.add_vars_with_literals(moves_n);
  LiteralVector dontshowmoves = cnf.add_vars_with_literals(moves_n-1);

  guint32 rows_n = level.get_rows_n();
  guint32 cols_n = level.get_columns_n();
  const guchar* blocks = level.get_blocks();

  gint32* plevelvars = new gint32[rows_n*cols_n*2];
  gint32* nlevelvars = new gint32[rows_n*cols_n*2];
  std::fill(plevelvars, plevelvars + 2*rows_n*cols_n, 0);

  gint32* dolefts = new gint32[rows_n*cols_n];
  gint32* dorights = new gint32[rows_n*cols_n];
  gint32* doups = new gint32[rows_n*cols_n];
  gint32* dodowns = new gint32[rows_n*cols_n];

  outmap_string.clear();

  /* invert fillmask excluding walls */
  for (guint32 i=0; i<rows_n*cols_n; i++)
    fill_mask[i] = (fill_mask[i] == 1 ||
	(blocks[i] & BLOCK_ENTITY_MASK) == BLOCK_WALL) ? 0 : 1;

  for (guint32 i=0; i<rows_n*cols_n; i++)
  {
     if (fill_mask[i]==0)
      continue;

     guchar b = blocks[i] & BLOCK_ENTITY_MASK;
     plevelvars[2*i] = cnf.add_var();
     plevelvars[2*i+1] = cnf.add_var();
     cnf.add_clause((b&1) ? plevelvars[2*i] : -plevelvars[2*i]);
     cnf.add_clause((b&2) ? plevelvars[2*i+1] : -plevelvars[2*i+1]);
  }

  for (guint32 move=0; move<moves_n; move++)
  {
    std::fill(dolefts, dolefts + rows_n*cols_n, 0);
    std::fill(dorights, dorights + rows_n*cols_n, 0);
    std::fill(doups, doups + rows_n*cols_n, 0);
    std::fill(dodowns, dodowns + rows_n*cols_n, 0);
    std::fill(nlevelvars, nlevelvars + 2*rows_n*cols_n, 0);

    LiteralVector movevar = cnf.add_vars_with_literals(2);
    gint32 doleft = cnf.add_var();
    gint32 doright = cnf.add_var();
    gint32 doup = cnf.add_var();
    gint32 dodown = cnf.add_var();

    if (with_outmap)
    {
      std::ostringstream os;
      if (move == 0)
	os << "{ {l|r|u|d} :" << movevar[0] << " " << movevar[1] << " " << " }";
      else
	os << "{ {l|r|u|d||||} :" << movevar[0] << " " << movevar[1] << " " <<
	    dontshowmoves[move-1] << " }";
      outmap_string += os.str();
    }

    cnf_gen_equal(cnf, doleft, movevar, 0);
    cnf_gen_equal(cnf, doright, movevar, 1);
    cnf_gen_equal(cnf, doup, movevar, 2);
    cnf_gen_equal(cnf, dodown, movevar, 3);

    for (guint32 y=0; y<rows_n; y++)
      for (guint32 x=0; x<cols_n; x++)
      {
	guint32 pos = y*cols_n+x;
	if (fill_mask[pos]==0)
	 continue;

	/* do left */
	if (fill_mask[pos-1] != 0)
	{
	  gint32 leftxy = cnf.add_var();
	  dolefts[pos] = leftxy;
	  if (x>=2 && fill_mask[pos-2] != 0)
	  { /* lxy_n <=> doleft ^ Bx,y_n=player ^ (Bx-1,y_n=empty V
	      (Bx-1,y_n=box ^ Bx-2,y=empty)) */
	    gint32 tmp0 = cnf.add_var();
	    gint32 tmp1 = cnf.add_var();
	    gint32 tmp2 = cnf.add_var();

	    cnf.add_clause(-leftxy, doleft);
	    cnf.add_clause(-leftxy, plevelvars[2*pos]);
	    cnf.add_clause(-leftxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-leftxy, tmp0);
	    gint32 clause[5] = { leftxy, -doleft,
		-plevelvars[2*pos], -plevelvars[2*pos+1], -tmp0 };
	    cnf.add_clause(5, clause);

	    cnf.add_clause(-tmp0, tmp1, tmp2);
	    cnf.add_clause(tmp0, -tmp1);
	    cnf.add_clause(tmp0, -tmp2);

	    cnf.add_clause(-tmp1, -plevelvars[2*pos-2]);
	    cnf.add_clause(-tmp1, -plevelvars[2*pos-1]);
	    cnf.add_clause(tmp1, plevelvars[2*pos-2], plevelvars[2*pos-1]);

	    cnf.add_clause(-tmp2, -plevelvars[2*pos-2]);	/* box */
	    cnf.add_clause(-tmp2, plevelvars[2*pos-1]);
	    cnf.add_clause(-tmp2, -plevelvars[2*pos-4]);	/* empty */
	    cnf.add_clause(-tmp2, -plevelvars[2*pos-3]);
	    gint32 clause2[5] = { tmp2, plevelvars[2*pos-2], -plevelvars[2*pos-1],
		plevelvars[2*pos-4], plevelvars[2*pos-3] };
	    cnf.add_clause(5, clause2);
	  }
	  else
	  { /* lxy_n <=> doleft ^ Bx,y_n=player ^ Bx-1,y_n=empty*/
	    cnf.add_clause(-leftxy, doleft);
	    cnf.add_clause(-leftxy, plevelvars[2*pos]);
	    cnf.add_clause(-leftxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-leftxy, -plevelvars[2*pos-2]);
	    cnf.add_clause(-leftxy, -plevelvars[2*pos-1]);

	    gint32 clause[6] = { leftxy, -doleft,
		-plevelvars[2*pos], -plevelvars[2*pos+1],
	      plevelvars[2*pos-2], plevelvars[2*pos-1] };
	    cnf.add_clause(6, clause);
	  }
	}

	/* do right */
	if (fill_mask[pos+1] != 0)
	{
	  gint32 rightxy = cnf.add_var();
	  dorights[pos] = rightxy;
	  if (x<cols_n-2 && fill_mask[pos+2] != 0)
	  { /* rxy_n <=> doright ^ Bx,y_n=player ^ (Bx+1,y_n=empty V
	      (Bx+1,y_n=box ^ Bx+2,y=empty)) */
	    gint32 tmp0 = cnf.add_var();
	    gint32 tmp1 = cnf.add_var();
	    gint32 tmp2 = cnf.add_var();

	    cnf.add_clause(-rightxy, doright);
	    cnf.add_clause(-rightxy, plevelvars[2*pos]);
	    cnf.add_clause(-rightxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-rightxy, tmp0);
	    gint32 clause[5] = { rightxy, -doright,
		-plevelvars[2*pos], -plevelvars[2*pos+1], -tmp0 };
	    cnf.add_clause(5, clause);

	    cnf.add_clause(-tmp0, tmp1, tmp2);
	    cnf.add_clause(tmp0, -tmp1);
	    cnf.add_clause(tmp0, -tmp2);

	    cnf.add_clause(-tmp1, -plevelvars[2*pos+2]);
	    cnf.add_clause(-tmp1, -plevelvars[2*pos+3]);
	    cnf.add_clause(tmp1, plevelvars[2*pos+2], plevelvars[2*pos+3]);

	    cnf.add_clause(-tmp2, -plevelvars[2*pos+2]);	/* box */
	    cnf.add_clause(-tmp2, plevelvars[2*pos+3]);
	    cnf.add_clause(-tmp2, -plevelvars[2*pos+4]);	/* empty */
	    cnf.add_clause(-tmp2, -plevelvars[2*pos+5]);
	    gint32 clause2[5] = { tmp2, plevelvars[2*pos+2], -plevelvars[2*pos+3],
		plevelvars[2*pos+4], plevelvars[2*pos+5] };
	    cnf.add_clause(5, clause2);
	  }
	  else
	  { /* rxy_n <=> doright ^ Bx,y_n=player ^ Bx+1,y_n=empty */
	    cnf.add_clause(-rightxy, doright);
	    cnf.add_clause(-rightxy, plevelvars[2*pos]);
	    cnf.add_clause(-rightxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-rightxy, -plevelvars[2*pos+2]);
	    cnf.add_clause(-rightxy, -plevelvars[2*pos+3]);

	    gint32 clause[6] = { rightxy, -doright,
		-plevelvars[2*pos], -plevelvars[2*pos+1],
	      plevelvars[2*pos+2], plevelvars[2*pos+3] };
	    cnf.add_clause(6, clause);
	  }
	}

	/* do up */
	if (fill_mask[pos-cols_n] != 0)
	{
	  gint32 upxy = cnf.add_var();
	  doups[pos] = upxy;
	  guint32 pos_1 = pos-cols_n;
	  guint32 pos_2 = pos-2*cols_n;

	  if (y>=2 && fill_mask[pos_2] != 0)
	  { /* uxy_n <=> doup ^ Bx,y_n=player ^ (Bx,y-1_n=empty V
	      (Bx,y-1_n=box ^ Bx,y-2=empty)) */
	    gint32 tmp0 = cnf.add_var();
	    gint32 tmp1 = cnf.add_var();
	    gint32 tmp2 = cnf.add_var();

	    cnf.add_clause(-upxy, doup);
	    cnf.add_clause(-upxy, plevelvars[2*pos]);
	    cnf.add_clause(-upxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-upxy, tmp0);
	    gint32 clause[5] = { upxy, -doup,
		-plevelvars[2*pos], -plevelvars[2*pos+1], -tmp0 };
	    cnf.add_clause(5, clause);

	    cnf.add_clause(-tmp0, tmp1, tmp2);
	    cnf.add_clause(tmp0, -tmp1);
	    cnf.add_clause(tmp0, -tmp2);

	    cnf.add_clause(-tmp1, -plevelvars[2*pos_1]);
	    cnf.add_clause(-tmp1, -plevelvars[2*pos_1+1]);
	    cnf.add_clause(tmp1, plevelvars[2*pos_1], plevelvars[2*pos_1+1]);

	    cnf.add_clause(-tmp2, -plevelvars[2*pos_1]);	/* box */
	    cnf.add_clause(-tmp2, plevelvars[2*pos_1+1]);
	    cnf.add_clause(-tmp2, -plevelvars[2*pos_2]);	/* empty */
	    cnf.add_clause(-tmp2, -plevelvars[2*pos_2+1]);
	    gint32 clause2[5] = { tmp2, plevelvars[2*pos_1], -plevelvars[2*pos_1+1],
		plevelvars[2*pos_2], plevelvars[2*pos_2+1] };
	    cnf.add_clause(5, clause2);
	  }
	  else
	  { /* uxy_n <=> doup ^ Bx,y_n=player ^ Bx,y-1_n=empty*/
	    cnf.add_clause(-upxy, doup);
	    cnf.add_clause(-upxy, plevelvars[2*pos]);
	    cnf.add_clause(-upxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-upxy, -plevelvars[2*pos_1]);
	    cnf.add_clause(-upxy, -plevelvars[2*pos_1+1]);

	    gint32 clause[6] = { upxy, -doup,
		-plevelvars[2*pos], -plevelvars[2*pos+1],
	      plevelvars[2*pos_1], plevelvars[2*pos_1+1] };
	    cnf.add_clause(6, clause);
	  }
	}

	/* do down */
	if (fill_mask[pos+cols_n] != 0)
	{
	  gint32 downxy = cnf.add_var();
	  dodowns[pos] = downxy;
	  guint32 pos_1 = pos+cols_n;
	  guint32 pos_2 = pos+2*cols_n;

	  if (y<rows_n-2 && fill_mask[pos_2] != 0)
	  { /* dxy_n <=> dodown ^ Bx,y_n=player ^ (Bx,y+1_n=empty V
	      (Bx,y+1_n=box ^ Bx,y+2=empty)) */
	    gint32 tmp0 = cnf.add_var();
	    gint32 tmp1 = cnf.add_var();
	    gint32 tmp2 = cnf.add_var();

	    cnf.add_clause(-downxy, dodown);
	    cnf.add_clause(-downxy, plevelvars[2*pos]);
	    cnf.add_clause(-downxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-downxy, tmp0);
	    gint32 clause[5] = { downxy, -dodown,
		-plevelvars[2*pos], -plevelvars[2*pos+1], -tmp0 };
	    cnf.add_clause(5, clause);

	    cnf.add_clause(-tmp0, tmp1, tmp2);
	    cnf.add_clause(tmp0, -tmp1);
	    cnf.add_clause(tmp0, -tmp2);

	    cnf.add_clause(-tmp1, -plevelvars[2*pos_1]);
	    cnf.add_clause(-tmp1, -plevelvars[2*pos_1+1]);
	    cnf.add_clause(tmp1, plevelvars[2*pos_1], plevelvars[2*pos_1+1]);

	    cnf.add_clause(-tmp2, -plevelvars[2*pos_1]);	/* box */
	    cnf.add_clause(-tmp2, plevelvars[2*pos_1+1]);
	    cnf.add_clause(-tmp2, -plevelvars[2*pos_2]);	/* empty */
	    cnf.add_clause(-tmp2, -plevelvars[2*pos_2+1]);
	    gint32 clause2[5] = { tmp2, plevelvars[2*pos_1], -plevelvars[2*pos_1+1],
		plevelvars[2*pos_2], plevelvars[2*pos_2+1] };
	    cnf.add_clause(5, clause2);
	  }
	  else
	  { /* dxy_n <=> dodown ^ Bx,y_n=player ^ Bx,y+1_n=empty*/
	    cnf.add_clause(-downxy, dodown);
	    cnf.add_clause(-downxy, plevelvars[2*pos]);
	    cnf.add_clause(-downxy, plevelvars[2*pos+1]);
	    cnf.add_clause(-downxy, -plevelvars[2*pos_1]);
	    cnf.add_clause(-downxy, -plevelvars[2*pos_1+1]);

	    gint32 clause[6] = { downxy, -dodown,
		-plevelvars[2*pos], -plevelvars[2*pos+1],
	      plevelvars[2*pos_1], plevelvars[2*pos_1+1] };
	    cnf.add_clause(6, clause);
	  }
	}
      }

    for (guint32 y=0; y<rows_n; y++)
      for (guint32 x=0; x<cols_n; x++)
      {
	guint32 pos = y*cols_n+x;
	if (fill_mask[pos]==0)
	  continue;

	guint32 posp1 = pos+cols_n;
	guint32 posp2 = pos+2*cols_n;
	guint32 posm1 = pos-cols_n;
	guint32 posm2 = pos-2*cols_n;

	if (fill_mask[pos+1] == 0 && fill_mask[pos-1] == 0 &&
            fill_mask[posp1] == 0 && fill_mask[posm1] == 0)
          continue;

	nlevelvars[2*pos] = cnf.add_var();
        nlevelvars[2*pos+1] = cnf.add_var();

        gint32 bempty = cnf.add_var();
        gint32 bplayer = cnf.add_var();
        gint32 bbox = cnf.add_var();

        /* for Bx,y_n+1 = player */
        {
          gint32 clause[5] = { -bplayer, dolefts[pos+1], dorights[pos-1],
	      doups[posp1], dodowns[posm1] };
	  cnf.add_clause(5, clause);
        }
	if (dolefts[pos+1] != 0)
	  cnf.add_clause(bplayer, -dolefts[pos+1]);
	if (dorights[pos-1] != 0)
	  cnf.add_clause(bplayer, -dorights[pos-1]);
	if (doups[posp1] != 0)
	  cnf.add_clause(bplayer, -doups[posp1]);
	if (dodowns[posm1] != 0)
	  cnf.add_clause(bplayer, -dodowns[posm1]);

	/* for Bx,y_n+1 = empty */
	gint32 tmp0 = cnf.add_var();
	cnf.add_clause(-tmp0, -plevelvars[2*pos]);
	cnf.add_clause(-tmp0, -plevelvars[2*pos+1]);
	cnf.add_clause(-tmp0, -bplayer);
	cnf.add_clause(tmp0, plevelvars[2*pos], plevelvars[2*pos+1], bplayer);

	{
          gint32 clause[6] = { -bempty, dolefts[pos], dorights[pos],
	      doups[pos], dodowns[pos], tmp0 };
	  cnf.add_clause(6, clause);
        }
	if (dolefts[pos] != 0)
	  cnf.add_clause(bempty, -dolefts[pos]);
	if (dorights[pos] != 0)
	  cnf.add_clause(bempty, -dorights[pos]);
	if (doups[pos] != 0)
	  cnf.add_clause(bempty, -doups[pos]);
	if (dodowns[pos] != 0)
	  cnf.add_clause(bempty, -dodowns[pos]);
	cnf.add_clause(bempty, -tmp0);

	/* for Bx,y_n+1 = box */
	tmp0 = cnf.add_var();
	cnf.add_clause(-tmp0, -plevelvars[2*pos]);
	cnf.add_clause(-tmp0, plevelvars[2*pos+1]);
	cnf.add_clause(-tmp0, -bplayer);
	cnf.add_clause(tmp0, plevelvars[2*pos], -plevelvars[2*pos+1], bplayer);

	gint32 ltmp = 0;
	gint32 rtmp = 0;
	gint32 utmp = 0;
	gint32 dtmp = 0;

	if (fill_mask[pos+1] != 0 && x < cols_n-2 && dolefts[pos+2] != 0)
	{
	  ltmp = cnf.add_var();
	  cnf.add_clause(-ltmp, -plevelvars[2*pos+2]);
	  cnf.add_clause(-ltmp, plevelvars[2*pos+3]);
	  cnf.add_clause(-ltmp, dolefts[pos+2]);
	  cnf.add_clause(ltmp, plevelvars[2*pos+2], -plevelvars[2*pos+3],
	      -dolefts[pos+2]);
	}

	if (fill_mask[pos-1] != 0 && x >= 2 && dorights[pos-2] != 0)
	{
	  rtmp = cnf.add_var();
	  cnf.add_clause(-rtmp, -plevelvars[2*pos-2]);
	  cnf.add_clause(-rtmp, plevelvars[2*pos-1]);
	  cnf.add_clause(-rtmp, dorights[pos-2]);
	  cnf.add_clause(rtmp, plevelvars[2*pos-2], -plevelvars[2*pos-1],
	      -dorights[pos-2]);
	}

	if (fill_mask[posp1] != 0 && y < rows_n-2 && doups[posp2] != 0)
	{
	  utmp = cnf.add_var();
	  cnf.add_clause(-utmp, -plevelvars[2*posp1]);
	  cnf.add_clause(-utmp, plevelvars[2*posp1+1]);
	  cnf.add_clause(-utmp, doups[posp2]);
	  cnf.add_clause(utmp, plevelvars[2*posp1], -plevelvars[2*posp1+1],
	      -doups[posp2]);
	}

	if (fill_mask[posm1] != 0 && y >= 2 && dodowns[posm2] != 0)
	{
	  dtmp = cnf.add_var();
	  cnf.add_clause(-dtmp, -plevelvars[2*posm1]);
	  cnf.add_clause(-dtmp, plevelvars[2*posm1+1]);
	  cnf.add_clause(-dtmp, dodowns[posm2]);
	  cnf.add_clause(dtmp, plevelvars[2*posm1], -plevelvars[2*posm1+1],
	      -dodowns[posm2]);
	}

	{
	  gint32 clause[6] = { -bbox, tmp0, ltmp, rtmp, utmp, dtmp };
	  cnf.add_clause(6, clause);
	}
	cnf.add_clause(bbox, -tmp0);
	if (ltmp != 0)
	  cnf.add_clause(bbox, -ltmp);
	if (rtmp != 0)
	  cnf.add_clause(bbox, -rtmp);
	if (utmp != 0)
	  cnf.add_clause(bbox, -utmp);
	if (dtmp != 0)
	  cnf.add_clause(bbox, -dtmp);

	/* Bx,y_n+1 = { bemptyVbbox, bplayer } */
	cnf.add_clause(nlevelvars[2*pos], bempty, bbox);
	cnf.add_clause(-nlevelvars[2*pos], -bempty);
	cnf.add_clause(-nlevelvars[2*pos], -bbox);

	cnf.add_clause(-nlevelvars[2*pos+1], bplayer, bbox);
	cnf.add_clause(nlevelvars[2*pos+1], -bplayer);
	cnf.add_clause(nlevelvars[2*pos+1], -bbox);
      }

    /* generate solved */
    for (guint32 i=0; i<cols_n*rows_n; i++)
      if ((blocks[i] & BLOCK_TARGET) != 0 && nlevelvars[2*i] != 0)
      {
	cnf.add_clause(-solveds[move], -nlevelvars[2*i]);
	cnf.add_clause(-solveds[move], nlevelvars[2*i+1]);
      }

    if (move != 0)
    {
      if (move == 1)
	cnf.add_clause(-dontshowmoves[0], solveds[0]);
      else
	cnf.add_clause(-dontshowmoves[move-1], dontshowmoves[move-2], solveds[move-1]);
    }

    std::swap(plevelvars, nlevelvars);
  }

  cnf.add_clause(solveds);

  delete[] dolefts;
  delete[] dorights;
  delete[] doups;
  delete[] dodowns;
  delete[] plevelvars;
  delete[] nlevelvars;
  delete[] fill_mask;
}
