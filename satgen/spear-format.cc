/*
 * spear-format.cc - set cover module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <cctype>
#include <map>
#include "gen-arith.h"
#include "spear-format.h"

using namespace SatUtils;

SpearFormatModule::SpearFormatModule ()
    : simplify (false)
{
  add_param ("simplify", "Simplify and propagate constants", simplify);
}

SpearFormatModule::~SpearFormatModule ()
{
}

void
SpearFormatModule::post_process_params ()
{
}

SatGenModule*
SpearFormatModule::create ()
{
  return static_cast<SatGenModule*>(new SpearFormatModule ());
}

namespace
{

static const guint reserved_names_n = 14;
static const std::string reserved_names[14] =
{ /* sorted */
  "conc", // 1
  "ite",  // 2
  "extr", // 3
  "sext", // 4
  "sge",  // 5
  "sgt",  // 6
  "sle",  // 7
  "slt",  // 8
  "trun", // 9
  "uge",  // 10
  "ugt",  // 11
  "ule",  // 12
  "ult",  // 13
  "zext", // 14
};

static const std::string spear_operator_names[SPEAROP_LAST+1] =
{
  "&", // SPEAROP_AND
  "|", // SPEAROP_OR
  "^", // SPEAROP_XOR
  "~", // SPEAROP_NOT
  "=>", // SPEARPRED_IMPL
  "=", // SPEARPRED_EQ
  "/=", // SPEARPRED_NE
  "ule", // SPEARPRED_ULE
  "uge", // SPEARPRED_UGE
  "ult", // SPEARPRED_ULT
  "ugt", // SPEARPRED_UGT
  "sle", // SPEARPRED_SLE
  "sge", // SPEARPRED_SGE
  "slt", // SPEARPRED_SLT
  "sgt", // SPEARPRED_SGT
  "ite", // SPEAROP_ITE
  "+", // SPEAROP_ADD
  "-", // SPEAROP_SUB
  "*", // SPEAROP_MUL
  "/u", // SPEAROP_UDIV
  "/s", // SPEAROP_SDIV
  "%u", // SPEAROP_UMOD
  "%s", // SPEAROP_SMOD
  "<<", // SPEAROP_SL
  ">>a", // SPEAROP_ASR
  ">>l", // SPEAROP_LSR
  "trun", // SPEAROP_TRUN
  "sext", // SPEAROP_SEXT
  "zext", // SPEAROP_ZEXT
  "conc", // SPEAROP_CONC
  "extr", // SPEAROP_EXTR
};

enum SpearArg
{
  SPEARARG_NONE = 0,
  SPEARARG_ANY,
  SPEARARG_BOOL,
  SPEARARG_INT8,
  SPEARARG_CONST8,
};

static const SpearArg spear_operator_args[SPEAROP_LAST+1][3] =
{
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_AND
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_OR
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_XOR
  { SPEARARG_ANY, SPEARARG_NONE, SPEARARG_NONE }, // SPEAROP_NOT
  { SPEARARG_BOOL, SPEARARG_BOOL, SPEARARG_NONE }, // SPEARPRED_IMPL
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_EQ
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_NE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_ULE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_UGE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_ULT
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_UGT
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_SLE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_SGE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_SLT
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEARPRED_SGT
  { SPEARARG_BOOL, SPEARARG_ANY, SPEARARG_ANY }, // SPEAROP_ITE
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_ADD
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_SUB
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_MUL
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_UDIV
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_SDIV
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_UMOD
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_SMOD
  { SPEARARG_ANY, SPEARARG_INT8, SPEARARG_NONE }, // SPEAROP_SL
  { SPEARARG_ANY, SPEARARG_INT8, SPEARARG_NONE }, // SPEAROP_ASR
  { SPEARARG_ANY, SPEARARG_INT8, SPEARARG_NONE }, // SPEAROP_LSR
  { SPEARARG_ANY, SPEARARG_NONE, SPEARARG_NONE }, // SPEAROP_TRUN
  { SPEARARG_ANY, SPEARARG_NONE, SPEARARG_NONE }, // SPEAROP_SEXT
  { SPEARARG_ANY, SPEARARG_NONE, SPEARARG_NONE }, // SPEAROP_ZEXT
  { SPEARARG_ANY, SPEARARG_ANY, SPEARARG_NONE }, // SPEAROP_CONC
  { SPEARARG_ANY, SPEARARG_CONST8, SPEARARG_CONST8 }, // SPEAROP_EXTR
};

};

std::string
SpearFormatModule::get_var_name (std::string::const_iterator& start,
    std::string::const_iterator end)
{
  std::string name;
  if  (start != end && isalpha (*start))
  {
    name.push_back (*start++);
    while (start != end && (isalnum (*start) || *start == '_'))
      name.push_back (*start++);
    if (std::binary_search (reserved_names,
          reserved_names + reserved_names_n, name))
       throw InputSyntaxError (lineno, "Used name of operator"
           " as variable is not permitted");
  }
  else
    throw InputSyntaxError (lineno, "Expected variable name");
  return name;
}


std::string
SpearFormatModule::get_normal_line (std::string::const_iterator& start,
    std::string::const_iterator end)
{
  std::string out;
  lineno++;
  while (start != end)
  {
    /* skip spaces */
    if (isspace (*start))
      while (start != end && *start != '\n' && isspace (*start))
        ++start;
    /* skip comments and empty lines */
    if (*start == '#' || *start == '\n')
    {
      start = std::find (start, end, '\n');
      start = (start != end) ? start+1 : end;
      lineno++;
      continue;
    }
    else /* if not empty line */
      break;
  }
  if (start != end)
  {
    std::string::const_iterator line_end = std::find (start, end, '\n');
    out.assign (start, line_end);
    /* set start to next line position */
    start = (line_end != end) ? line_end+1 : end;
  }

  return out;
}

guint
SpearFormatModule::get_integer_type (std::string::const_iterator& start,
    std::string::const_iterator end)
{
  guint bits;
  if (start == end || *start != ':')
    throw InputSyntaxError (lineno, "Expected colon");
  ++start;
  if (start == end || *start != 'i')
    throw InputSyntaxError (lineno, "Expected type identifier");
  ++start;
  {
    std::istringstream is (std::string (start, end));
    is >> bits;
    if (is.fail ())
      throw InputSyntaxError (lineno, "Cant parse number of bits");
    if (bits > 64 || bits == 0)
      throw InputSyntaxError (lineno, "Bits number must be in range 1-64");
    start += is.tellg (); /* next position */
  }
  /* must be space after bits number if not end of line */
  if (start != end && !isspace (*start))
    throw InputSyntaxError (lineno,
	"Must be spaces between variable definitions");
  return bits;
}

void
SpearFormatModule::get_operand (std::string::const_iterator& start,
    std::string::const_iterator end, SpearOperand& operand)
{
  /* skip spaces */
  while (start != end && isspace (*start))
    start++;
  if (start != end)
  {
    if (isdigit (*start))
    {
      std::istringstream is (std::string (start, end));
      is >> operand.value;
      start += is.tellg ();
      operand.bits = get_integer_type (start, end);
      operand.constant = true;
    }
    else if (isalpha (*start) || *start == '_')
    {
      std::string name = get_var_name (start, end);
      if (varids.find (name) == varids.end ())
	throw InputSyntaxError (lineno, "Variable is not defined");
      operand.varid = varids[name];
      operand.bits = var_sizes[operand.varid];
      operand.constant = false;
    }
    else
      throw InputSyntaxError (lineno, "Expected numeral or variable name");
  }
  else
    throw InputSyntaxError (lineno, "Expected numeral or variable name");
}


void
SpearFormatModule::parse_input (const std::string& input)
{
  guint vars_n = 0;
  lineno = 0;
  varids.clear ();
  var_sizes.clear ();
  predicates.clear ();
  constraints.clear ();

  std::string::const_iterator input_it = input.begin ();

  { /* version format line */
    std::istringstream is (get_normal_line (input_it, input.end ()));
    char first_char = is.get ();
    if (!is.eof () && first_char == 'v')
    {
      std::string version;
      is >> version;
      if (version != "1.0")
	throw InputSyntaxError (lineno, "Bad version of format");
    }
    else
      throw InputSyntaxError (lineno, "First line must be a version line");
  }

  /* ignore expect line */
  std::string line = get_normal_line (input_it, input.end ());
  if (line.size () != 0 && line[0] == 'e')
    line = get_normal_line (input_it, input.end ());
  /* definitions of variables */
  while (line.size () != 0)
  {
    if (line[0] == 'd')
    {
      std::string::const_iterator line_it = line.begin () + 1;
      while (line_it != line.end ())
      {
        std::string name;
        guint bits;
        /* skip spaces */
        while (line_it != line.end () && isspace (*line_it))
          ++line_it;
        /* get name */
        name = get_var_name (line_it, line.end ());
        bits = get_integer_type (line_it, line.end ());

        if (varids.find (name) != varids.end ())
          throw InputSyntaxError (lineno,
              "Multiple definitions of variable is not legal");
        /* set var id */
        varids[name] = vars_n++;
        var_sizes.push_back (bits);
      }
    }
    else
      break; /* is not definition line */
    line = get_normal_line (input_it, input.end ());
  }

  /* constraints and predicates */
  while (line.size () != 0)
  {
    if (line[0] == 'c')
    { /* is constraints */
      SpearConstraint constraint;
      std::string::const_iterator line_it = line.begin () + 1;
      get_operand (line_it, line.end (), constraint.result);

      /* skip spaces */
      while (line_it != line.end () && isspace (*line_it))
        ++line_it;

      std::string opname;
      while (line_it != line.end () && !isspace (*line_it))
        opname += *line_it++;
      const std::string* found = std::find (spear_operator_names,
          spear_operator_names + SPEAROP_LAST+1, opname);
      if (found != spear_operator_names + SPEAROP_LAST + 1) // is found
        /* number of operator */
        constraint.op = SpearOp (found - spear_operator_names);
      else
        throw InputSyntaxError (lineno, "Bad operator in constraint");
      /* parse operands */
      const SpearArg* operator_args = spear_operator_args[constraint.op];
      for (guint i = 0; i < 3; i++)
	if (operator_args[i] != SPEARARG_NONE)
	{
	  get_operand (line_it, line.end (), constraint.args[i]);
	  if (operator_args[i] == SPEARARG_BOOL && constraint.args[i].bits != 1)
	    throw InputSyntaxError (lineno, "Operand must be of type i1");
	  else if (operator_args[i] == SPEARARG_CONST8 &&
	      (constraint.args[i].bits != 8 || !constraint.args[i].constant))
	    throw InputSyntaxError (lineno, "Operand must be constant of type i8");
	  else if (operator_args[i] == SPEARARG_INT8 &&
	           constraint.args[i].bits != 8)
	    throw InputSyntaxError (lineno, "Operand must be of type i8");
	}
	else
	  constraint.args[i].bits = 0;

      /* skip spaces */
      while (line_it != line.end () && isspace (*line_it))
	++line_it;
      if (line_it != line.end ())
	throw InputSyntaxError (lineno, "Extra operand in constraint");

      /* check for this same type */
      {
        if (constraint.op != SPEAROP_CONC)
        {
          guint bits1 = 0, bits2 = 0;
	  if (operator_args[0] == SPEARARG_ANY)
	    bits1 = constraint.args[0].bits;
	  if (operator_args[1] == SPEARARG_ANY)
	  {
	    if (bits1 == 0)
	      bits1 = constraint.args[1].bits;
	    else
	      bits2 = constraint.args[1].bits;
	  }
	  if (operator_args[2] == SPEARARG_ANY)
	  {
	    if (bits1 == 0)
	      bits1 = constraint.args[2].bits;
	    else
	      bits2 = constraint.args[2].bits;
	  }
	  if (bits1 != 0 && bits2 != 0 && bits1 != bits2)
	    throw InputSyntaxError (lineno,
	      "Two operands in constraints must be with same type");
        }

        if (constraint.op >= SPEARPRED_FIRST &&
            constraint.op <= SPEARPRED_LAST &&
            constraint.result.bits != 1)
          throw InputSyntaxError (lineno,
              "In predicate operator result must be boolean");
        if (constraint.op == SPEAROP_TRUN &&
            constraint.args[0].bits <= constraint.result.bits)
         throw InputSyntaxError (lineno,
              "Truncate operator requires larger operand than result");
        if ((constraint.op == SPEAROP_SEXT ||
             constraint.op == SPEAROP_ZEXT) &&
             constraint.args[0].bits >= constraint.result.bits)
         throw InputSyntaxError (lineno,
              "SEXT or ZEXT requires smaller operand than result");
        if (constraint.op == SPEAROP_CONC &&
            constraint.args[0].bits + constraint.args[1].bits !=
                constraint.result.bits)
         throw InputSyntaxError (lineno,
              "In CONC result size must be sum of size of two operands");
        if (constraint.op == SPEAROP_EXTR)
        {
          if (constraint.args[2].value <= constraint.args[1].value)
            throw InputSyntaxError (lineno,
              "In EXTR third operand must be greater than second");
          guint req_result_bits = constraint.args[2].value -
              constraint.args[1].value;
          if (req_result_bits != constraint.result.bits)
            throw InputSyntaxError (lineno,
              "In EXTR result size must be difference between"
              " 3rd and 2nd operand.");
          if (constraint.args[1].value >= constraint.args[0].bits ||
              constraint.args[2].value > constraint.args[0].bits)
            throw InputSyntaxError (lineno,
              "In EXTR 2nd and 3rd operand values must be not greater than"
              " size of first");
        }
      }

      constraints.push_back (constraint);
    }
    else if (line[0] == 'p')
    { /* is predicate */
      SpearPredicate predicate;

      std::string::const_iterator line_it = line.begin () + 1;
      std::string opname;

      /* skip spaces */
      while (line_it != line.end () && isspace (*line_it))
        ++line_it;

      while (line_it != line.end () && !isspace (*line_it))
        opname += *line_it++;
      const std::string* found = std::find (
          spear_operator_names, spear_operator_names + SPEAROP_LAST+1, opname);

      if (found != spear_operator_names + SPEAROP_LAST + 1) // is found
      {
        /* number of operator */
        predicate.pred = SpearOp (found - (spear_operator_names));
        if ((predicate.pred < SPEARPRED_FIRST ||
             predicate.pred > SPEARPRED_LAST) &&
            (predicate.pred != SPEAROP_AND &&
              predicate.pred != SPEAROP_OR &&
              predicate.pred != SPEAROP_XOR &&
              predicate.pred != SPEAROP_NOT &&
              predicate.pred != SPEAROP_ITE))
          throw InputSyntaxError (lineno,
             "Only predicates and 1-bit AND,OR,XOR,NOT,ITE is allowed");
      }
      else
        throw InputSyntaxError (lineno, "Bad operator in predicate");

      /* parse operands */
      const SpearArg* operator_args = spear_operator_args[predicate.pred];
      for (guint i = 0; i < 3; i++)
        if (operator_args[i] != SPEARARG_NONE)
          get_operand (line_it, line.end (), predicate.args[i]);
        else
          predicate.args[i].bits = 0;

      /* skip spaces */
      while (line_it != line.end () && isspace (*line_it))
	++line_it;
      if (line_it != line.end ())
	throw InputSyntaxError (lineno, "Extra operand in predicate");

      if (predicate.pred == SPEAROP_ITE)
      {
        if (predicate.args[0].bits != 1)
          throw InputSyntaxError (lineno, "Operand must be of type i1");
        if (predicate.args[1].bits != 1)
          throw InputSyntaxError (lineno,
	      "In predicate other operators must be with 1-bit operands");
      }
      else if ((predicate.pred < SPEARPRED_FIRST ||
		predicate.pred > SPEARPRED_LAST) &&
		predicate.args[0].bits != 1) /* is not predicate */
	throw InputSyntaxError (lineno,
	    "In predicate other operators must be with 1-bit operands");

      if ((predicate.args[1].bits != 0 &&
           predicate.args[0].bits != predicate.args[1].bits) ||
          (predicate.args[2].bits != 0 &&
           predicate.args[0].bits != predicate.args[2].bits))
        throw InputSyntaxError (lineno,
            "Operands in predicate must be with same type");

      predicates.push_back (predicate);
    }
    else
      throw InputSyntaxError (lineno, "Unknown line type");

    line = get_normal_line (input_it, input.end ());
  }
}

namespace
{

/* replacement for ((1ULL<<bits)-1), prevent bug when bits is 64
  (when output is not G_MAXUINT64) */
inline guint64 lomask (guint bits)
{
  return (bits < 64) ? ((1ULL<<bits)-1) : G_MAXUINT64;
}

/* replacemenent for (1ULL<<bits), prevent bug whene bits is 64
   (when output is not zero) */
inline guint64 bitval (guint bits)
{
  return (bits < 64) ? 1ULL<<bits : 0;
}

inline guint64 shl64 (guint64 v, guint bits)
{
  return (bits < 64) ? v<<bits : 0;
}
inline guint64 shr64 (guint64 v, guint bits)
{
  return (bits < 64) ? v>>bits : 0;
}

/* find nearest inverse: A*X=GCD(A,B) <=> {[X],Y}=ExtendedGCD(A,B)
   b=0 <=> B=2^64; returns A */
static guint64
nearest_inverse (guint64 a, guint64 b, guint64& gcd)
{
  guint64 xm2 = 1;
  guint64 ym2 = 0;
  guint64 a0 = a;
  guint64 b0 = b;
  guint64 temp, div;
  guint64 x0 = 0;
  guint64 y0 = 1;

  /* if B=2^64 */
  if (b0 == 0)
  {
    temp = b0;
    div = 0;
    b0 = a0;
    //a0 = temp; a0=2^64
    //std::cout << "div:" << div << ",mod:" << b0 << std::endl;

    temp = x0;
    x0 = xm2 /*- div*x0*/;
    xm2 = temp;
    temp = y0;
    y0 = ym2 /*- div*y0*/;
    ym2 = temp;

    if (b0 != 0)
    {
      temp = b0;
      div = a0 / b0;
      {
        div = G_MAXUINT64 / b0;
        guint64 mod = G_MAXUINT64 - div*b0;
        if (mod == b0-1)
          div++;
        b0 = (mod+1 < b0) ? mod+1 : 0;
      }
      //b0 = a0 % b0;
      a0 = temp;
      //std::cout << "div:" << div << ",mod:" << b0 << std::endl;

      temp = x0;
      x0 = xm2 - div*x0;
      xm2 = temp;
      temp = y0;
      y0 = ym2 - div*y0;
      ym2 = temp;
    }
  }

  while (b0 != 0)
  {
    temp = b0;
    div = a0 / b0;
    b0 = a0 % b0;
    a0 = temp;
    //std::cout << "div:" << div << ",mod:" << b0 << std::endl;

    temp = x0;
    x0 = xm2 - div*x0;
    xm2 = temp;
    temp = y0;
    y0 = ym2 - div*y0;
    ym2 = temp;
  }

  gcd = a0;
  return xm2;
}

/* modulo_div - modulo division (c=a/b <=> a=(b*c mod (2^bits)),
   shift - number of bits beginning of end result which
   may be ignored in comparison  */

static guint64
modulo_div (guint64 a, guint64 b, guint bits, guint& shift, bool& isfound)
{
  guint64 div = 0;
  if (b != 0)
  {
    div = a/b;
    guint64 gcd = 0;
    guint64 realmod = a-div*b;
    guint64 inverse = nearest_inverse (b, bitval (bits), gcd);
    shift = 0;
    /* check if realmod-(inverse*x mod b)=0 is satisfiable (where b is 2^n) */
    while ((gcd&1) == 0 && (realmod&1) == 0)
    {
      gcd >>= 1;
      realmod >>= 1;
      shift++;
    }
    if ((gcd&1) | (1^(realmod&1)))
    {
      div = (div + realmod*inverse) & lomask (bits);
      isfound = true;
    }
    else
      isfound = false;
  }
  else
  {
    isfound = false;
    return 0;
  }
  return div;
}

static guint64 multiply64 (guint64 x, guint64 y, guint64& higher)
{
  guint64 lower;
  guint32 xz = x;
  guint32 yz = y;
  guint32 xa = x>>32;
  guint32 ya = y>>32;
  guint64 tmp1 = guint64(xa)*yz;
  guint64 tmp2 = guint64(ya)*xz;
  lower = guint64(xz)*yz;
  guint64 adc = (lower>>32)+guint32(tmp1)+guint32(tmp2);
  lower = (lower & 0xffffffffULL) | (adc << 32);
  higher = guint64(xa)*ya + (adc>>32) + (tmp1>>32)+(tmp2>>32);
  return lower;
}


static void
generate_from_constraints (CNF& cnf, const SpearConstraintVector& constraints,
    const std::vector<LiteralVector>& vars_literals, gint32 zerovar)
{
  for (SpearConstraintConstIter con = constraints.begin ();
       con != constraints.end (); ++con)
  {
    const LiteralVector* result = 0;
    const LiteralVector* arg1 = 0;
    const LiteralVector* arg2 = 0;
    const LiteralVector* arg3 = 0;
    guint64 resultval = 0;
    guint64 arg1val = 0;
    guint64 arg2val = 0;
    guint64 arg3val = 0;

    if (!con->result.constant)
      result = &(vars_literals[con->result.varid]);
    else
      resultval = (con->result.value) & lomask (con->result.bits);

    if (con->args[0].bits != 0)
    {
      if (!con->args[0].constant)
	arg1 = &(vars_literals[con->args[0].varid]);
      else
	arg1val = con->args[0].value & lomask (con->args[0].bits);
    }

    if (con->args[1].bits != 0)
    {
      if (!con->args[1].constant)
	arg2 = &(vars_literals[con->args[1].varid]);
      else
	arg2val = con->args[1].value & lomask (con->args[1].bits);
    }

    if (con->args[2].bits != 0)
    {
      if (!con->args[2].constant)
	arg3 = &(vars_literals[con->args[2].varid]);
      else
	arg3val = con->args[2].value & lomask (con->args[2].bits);
    }

    /* mask for result, used in constraint generation */
    guint64 mask = lomask (con->result.bits);

    switch (con->op)
    {
      case SPEAROP_AND:
        if (result)
        { /* if not value */
          if (arg1)
          {
            if (arg2)
              cnf_gen_and (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_and (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_and (cnf, *result, *arg2, arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val & arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_and (cnf, resultval, *arg1, *arg2);
            else
              cnf_gen_and (cnf, resultval, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_and (cnf, resultval, *arg2, arg1val);
            else if (((resultval)^(arg1val & arg2val)) != 0)
              /* if bits or a ^ b is not equal with result */
              cnf.add_clause (0); /* unsat */
          }
        }
        break;
      case SPEAROP_OR:
        if (result)
        { /* if not value */
          if (arg1)
          {
            if (arg2)
              cnf_gen_or (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_or (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_or (cnf, *result, *arg2, arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val | arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_or (cnf, resultval, *arg1, *arg2);
            else
              cnf_gen_or (cnf, resultval, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_or (cnf, resultval, *arg2, arg1val);
            else if (((resultval)^(arg1val | arg2val)) != 0)
              /* if bits or a ^ b is not equal with result */
              cnf.add_clause (0); /* unsat */
          }
        }
        break;
      case SPEAROP_XOR:
        if (result)
        { /* if not value */
          if (arg1)
          {
            if (arg2)
              cnf_gen_xor (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_xor (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_xor (cnf, *result, *arg2, arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val ^ arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            if (arg2)
            { /* res = a xor b <=> -res xor a xor b <=> -a = -res xor b */
              LiteralVector negarg1 = *arg1;
              for (LiteralIter lit = negarg1.begin (); lit != negarg1.end (); ++lit)
                *lit = -*lit;
              cnf_gen_xor (cnf, negarg1, *arg2, ~resultval);
            }
            else
              cnf_gen_equal (cnf, 0, *arg1, (resultval^arg2val), DEF_POSITIVE);
          }
          else
          {
            if (arg2)
              cnf_gen_equal (cnf, 0, *arg2, (resultval^arg1val), DEF_POSITIVE);
            else if (((resultval)^(arg1val ^ arg2val)) != 0)
              /* if bits or a ^ b is not equal with result */
              cnf.add_clause (0); /* unsat */
          }
        }
        break;
      case SPEAROP_NOT:
        if (result)
        {
          if (arg1)
            cnf_gen_xor (cnf, *result, *arg1, G_MAXUINT64);
          else
            cnf_gen_equal (cnf, 0, *result, ~arg1val, DEF_POSITIVE);
        }
        else
        {
          if (arg1)
            /* out = -a <=> -out xor -a <=> out=-a */
            cnf_gen_equal (cnf, 0, *arg1, ~resultval, DEF_POSITIVE);
          else if (((~resultval^arg1val) & mask) != 0)
            cnf.add_clause (0);
        }
        break;
      case SPEARPRED_IMPL:
        if (result)
        { /* if not value */
          if (arg1)
          {
            LiteralVector negarg1;
            negarg1.push_back (-(*arg1)[0]);

            if (arg2)
              cnf_gen_or (cnf, *result, negarg1, *arg2);
            else
              cnf_gen_or (cnf, *result, negarg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_or (cnf, *result, *arg2, ~arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, ~arg1val | arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            LiteralVector negarg1;
            negarg1.push_back (-(*arg1)[0]);

            if (arg2)
              cnf_gen_or (cnf, resultval, negarg1, *arg2);
            else
              cnf_gen_or (cnf, resultval, negarg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_or (cnf, resultval, *arg2, ~arg1val);
            else if ((((resultval)^(~arg1val | arg2val)) & mask) != 0)
              /* if bits or a ^ b is not equal with result */
              cnf.add_clause (0); /* unsat */
          }
        }
        break;
      case SPEARPRED_EQ:
      case SPEARPRED_NE:
        if (result)
	{
	  gint32 resvar = (con->op == SPEARPRED_EQ) ? (*result)[0] :
	      -(*result)[0];
	  if (arg1)
	  {
	    if (arg2)
	      cnf_gen_equal (cnf, resvar, *arg1, *arg2, DEF_FULL);
	    else
	      cnf_gen_equal (cnf, resvar, *arg1, arg2val, DEF_FULL);
	  }
	  else
	  {
	    if (arg2)
	      cnf_gen_equal (cnf, resvar, *arg2, arg1val, DEF_FULL);
	    else /* if not equal */
	      cnf.add_clause ((arg1val == arg2val) ?
		  resvar : -resvar);
	  }
	}
	else
	{
	  resultval = (con->op == SPEARPRED_NE) ? resultval ^ 1 : resultval;
	  DefType def_type = ((resultval&1) != 0) ? DEF_POSITIVE : DEF_NEGATIVE;
	  if (arg1)
	  {
	    if (arg2)
	      cnf_gen_equal (cnf, 0, *arg1, *arg2, def_type);
	    else
	      cnf_gen_equal (cnf, 0, *arg1, arg2val, def_type);
	  }
	  else
	  {
	    if (arg2)
	      cnf_gen_equal (cnf, 0, *arg2, arg1val, def_type);
	    else if ((arg1val != arg2val) == ((resultval&1) != 0))
	      /* a==b and result=0 => unsat,
		a!=b and result=1 => unsat */
	      cnf.add_clause (0);
	  }
	}
	break;
      case SPEARPRED_ULE:
      case SPEARPRED_UGT:
      case SPEARPRED_SLE:
      case SPEARPRED_SGT:
        {
	  bool is_signed = (con->op == SPEARPRED_SLE || con->op == SPEARPRED_SGT);

	  /* value used to reverse sign */
	  guint64 sign = (is_signed) ? (1ULL << (con->args[1].bits-1)) : 0;

	  if (result)
	  {
	    gint32 resvar = (con->op == SPEARPRED_ULE ||
	       con->op == SPEARPRED_SLE) ? (*result)[0] : -(*result)[0];
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_lesseq (cnf, resvar, *arg1, *arg2,
		    DEF_FULL, is_signed);
	      else
		cnf_gen_lesseq (cnf, resvar, *arg1, arg2val,
		    DEF_FULL, is_signed);
	    }
	    else
	    {
	      if (arg2) /* !(b<a) <=> b=>a <=> a<=b */
		cnf_gen_less (cnf, -resvar, *arg2, arg1val,
		    DEF_FULL, is_signed);
	      else
	        cnf.add_clause (((arg1val^sign) <= (arg2val^sign)) ?
		      resvar : -resvar);
	    }
	  }
	  else
	  {
	    resultval = (con->op == SPEARPRED_ULE || con->op == SPEARPRED_SLE) ?
	        resultval : resultval^1;
	    DefType def_type = (resultval&1) ? DEF_POSITIVE : DEF_NEGATIVE;
	    DefType neg_def_type = (resultval&1) ? DEF_NEGATIVE: DEF_POSITIVE;
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_lesseq (cnf, 0, *arg1, *arg2, def_type, is_signed);
	      else
		cnf_gen_lesseq (cnf, 0, *arg1, arg2val, def_type, is_signed);
	    }
	    else
	    {
	      if (arg2) /* !(b<a) <=> b=>a <=> a<=b */
		cnf_gen_less (cnf, 0, *arg2, arg1val, neg_def_type, is_signed);
	      else if (((arg1val^sign) > (arg2val^sign)) == ((resultval&1) != 0))
		/* a<=b and result=0 => unsat,
		  a>b and result=1 => unsat */
		cnf.add_clause (0);
	    }
	  }
        }
        break;
      case SPEARPRED_ULT:
      case SPEARPRED_UGE:
      case SPEARPRED_SLT:
      case SPEARPRED_SGE:
        {
          bool is_signed = (con->op == SPEARPRED_SLT || con->op == SPEARPRED_SGE);
          LiteralVector s_arg1, s_arg2;
          guint64 s_arg1val = 0, s_arg2val = 0;
          /* negate last bit of arguments if signed comparison */
          if (arg1)
          {
            s_arg1 = *arg1;
            if (is_signed)
              s_arg1[s_arg1.size ()-1] = -s_arg1[s_arg1.size ()-1];
          }
          else
            s_arg1val = (is_signed) ?
                arg1val ^ (1ULL << (con->args[0].bits-1)) : arg1val;
          if (arg2)
          {
            s_arg2 = *arg2;
            if (is_signed)
              s_arg2[s_arg2.size ()-1] = -s_arg2[s_arg2.size ()-1];
          }
          else
            s_arg2val = (is_signed) ?
                arg2val ^ (1ULL << (con->args[1].bits-1)) : arg2val;

	  if (result)
	  {
	    gint32 resvar = (con->op == SPEARPRED_ULT ||
	         con->op == SPEARPRED_SLT) ? (*result)[0] : -(*result)[0];
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_less (cnf, resvar, s_arg1, s_arg2, DEF_FULL);
	      else
		cnf_gen_less (cnf, resvar, s_arg1, s_arg2val, DEF_FULL);
	    }
	    else
	    {
	      if (arg2) /* !(b<=a) <=> b>a <=> a<b */
		cnf_gen_lesseq (cnf, -resvar, s_arg2, s_arg1val, DEF_FULL);
	      else
		cnf.add_clause ((s_arg1val < s_arg2val) ?
		    resvar : -resvar);
	    }
	  }
	  else
	  {
	    resultval = (con->op == SPEARPRED_ULT || con->op == SPEARPRED_SLT) ?
	        resultval : resultval^1;
	    DefType def_type = (resultval&1) ? DEF_POSITIVE : DEF_NEGATIVE;
	    DefType neg_def_type = (resultval&1) ? DEF_NEGATIVE: DEF_POSITIVE;
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_less (cnf, 0, s_arg1, s_arg2, def_type);
	      else
		cnf_gen_less (cnf, 0, s_arg1, s_arg2val, def_type);
	    }
	    else
	    {
	      if (arg2) /* !(b<=a) <=> b>a <=> a<b */
		cnf_gen_lesseq (cnf, 0, s_arg2, s_arg1val, neg_def_type);
	      else if ((s_arg1val >= s_arg2val) == ((resultval&1) != 0))
		/* a<b and result=0 => unsat,
		  a>=b and result=1 => unsat */
		cnf.add_clause (0);
	    }
	  }
        }
        break;
      case SPEAROP_ITE:
        if (result)
        {
          if (arg1) /* cond is variable */
          {
            gint32 cond = (*arg1)[0];
            if (arg2)
            {
              if (arg3)
                cnf_gen_ite (cnf, *result, cond, *arg2, *arg3);
              else
                cnf_gen_ite (cnf, *result, cond, *arg2, arg3val);
            }
            else
            {
              if (arg3)
                cnf_gen_ite (cnf, *result, -cond, *arg3, arg2val);
              else
                cnf_gen_ite (cnf, *result, cond, arg2val, arg3val);
            }
          }
          else
          {
            guint64 sel_argval = (arg1val&1) ? arg2val : arg3val;
            const LiteralVector* sel_arg = (arg1val&1) ? arg2 : arg3;
            if (sel_arg)
              cnf_gen_equal (cnf, 0, *result, *sel_arg, DEF_POSITIVE);
            else
              cnf_gen_equal (cnf, 0, *result, sel_argval, DEF_POSITIVE);
          }
        }
        else /* result is known */
        {
          if (arg1)
          {
            gint32 cond = (*arg1)[0];
            if (arg2)
            {
              if (arg3)
                cnf_gen_ite (cnf, resultval, cond, *arg2, *arg3);
              else
                cnf_gen_ite (cnf, resultval, cond, *arg2, arg3val);
            }
            else
            {
              if (arg3)
                cnf_gen_ite (cnf, resultval, -cond, *arg3, arg2val);
              else
              {
                if (resultval == (arg2val))
                {
                  if (resultval != (arg3val))
                    cnf.add_clause (cond); /* cond must be true */
                  /* else might be true or false */
                }
                else
                {
                  if (resultval == (arg3val))
                    cnf.add_clause (-cond); /* cond must be false */
                  else
                    cnf.add_clause (0); /* unsatisfiable */
                }
              }
            }
          }
          else
          {
            guint64 sel_argval = (arg1val&1) ? arg2val : arg3val;
            const LiteralVector* sel_arg = (arg1val&1) ? arg2 : arg3;
            if (sel_arg)
              cnf_gen_equal (cnf, 0, *sel_arg, resultval, DEF_POSITIVE);
            else if ((resultval) != (sel_argval))
              cnf.add_clause (0); /* unsatisfiable */
          }
        }
        break;
      case SPEAROP_ADD:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_add (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_add (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_add (cnf, *result, *arg2, arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val+arg2val, DEF_POSITIVE);
          }
        }
        else /* result is constant */
        {
          if (arg1)
          {
            if (arg2) /* res=a+b <=> res-b=a */
              cnf_gen_sub (cnf, *arg1, resultval, *arg2);
            else
              cnf_gen_equal (cnf, 0, *arg1, resultval-arg2val, DEF_POSITIVE);
          }
          else
          {
            if (arg2)
              cnf_gen_equal (cnf, 0, *arg2, resultval-arg1val, DEF_POSITIVE);
            else if (resultval != ((arg1val+arg2val)&mask))
              cnf.add_clause (0);
          }
        }
        break;
      case SPEAROP_SUB:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_sub (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_sub (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_sub (cnf, *result, arg1val, *arg2);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val-arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            if (arg2) /* res=a-b <=> res+b=a */
              cnf_gen_add (cnf, *arg1, *arg2, resultval);
            else
              cnf_gen_equal (cnf, 0, *arg1, (resultval+arg2val)&mask, DEF_POSITIVE);
          }
          else
          {
            if (arg2) /* res=a-b <=> res-a=-b <=> b=a-res */
              cnf_gen_equal (cnf, 0, *arg2, arg1val-resultval, DEF_POSITIVE);
            else if (resultval != ((arg1val-arg2val)&mask))
              cnf.add_clause (0);
          }
        }
        break;
      case SPEAROP_MUL:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_uint_mul (cnf, *result, *arg1, *arg2);
            else
              cnf_gen_uint_mul (cnf, *result, *arg1, arg2val);
          }
          else
          {
            if (arg2)
              cnf_gen_uint_mul (cnf, *result, *arg2, arg1val);
            else
              cnf_gen_equal (cnf, 0, *result, arg1val*arg2val, DEF_POSITIVE);
          }
        }
        else
        {
          LiteralVector constresult (con->result.bits);
	  for (guint i = 0; i < con->result.bits; i++)
	    constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;
          if (arg1)
          {
            if (arg2)
              cnf_gen_uint_mul (cnf, constresult, *arg1, *arg2);
            else
            {
              //cnf_gen_uint_mul (cnf, constresult, *arg1, arg2val);
              bool isfound;
              guint ignored_bits = 0;
              guint64 div = modulo_div (resultval, arg2val, con->result.bits,
                  ignored_bits, isfound);
              if (isfound)
              {
                LiteralVector temparg1 (arg1->begin (), arg1->end () - ignored_bits);
                cnf_gen_equal (cnf, 0, temparg1, div, DEF_POSITIVE);
              }
              /* if arg2 is zero and if result is zero then is satisfiable */
              else if (arg2val != 0 || resultval != 0)
                cnf.add_clause (0);
            }
          }
          else
          {
            if (arg2)
            {
              //cnf_gen_uint_mul (cnf, constresult, *arg2, arg1val);
              bool isfound;
              guint ignored_bits = 0;
              guint64 div = modulo_div (resultval, arg1val, con->result.bits,
                  ignored_bits, isfound);
              if (isfound)
              {
                LiteralVector temparg2 (arg2->begin (), arg2->end () - ignored_bits);
                cnf_gen_equal (cnf, 0, temparg2, div, DEF_POSITIVE);
              }
              /* if arg1 is zero and if result is zero then is satisfiable */
              else if (arg1val != 0 || resultval != 0)
                cnf.add_clause (0);
            }
            else if (resultval != ((arg1val*arg2val)&mask))
              cnf.add_clause (0);
          }
        }
        break;
      case SPEAROP_UDIV:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_uint_divmod (cnf, true, *result, *arg1, *arg2);
            else if (arg2val != 0)
              cnf_gen_uint_divmod (cnf, true, *result, *arg1, arg2val);
            else
              cnf.add_clause (0);
          }
          else
          {
            if (arg2)
            { /* a/b, a - is known */
              LiteralVector constarg1 (con->args[0].bits);
              for (guint i = 0; i < con->args[0].bits; i++)
                constarg1[i] = (arg1val&(1ULL<<i)) ? -zerovar : zerovar;
              cnf_gen_uint_divmod (cnf, true, *result, constarg1, *arg2);
            }
            else if (arg2val != 0)
              cnf_gen_equal (cnf, 0, *result, arg1val/arg2val, DEF_POSITIVE);
            else
              cnf.add_clause (0);
          }
        }
        else
        {
          if (arg1)
          {
            LiteralVector constresult (con->result.bits);
	    for (guint i = 0; i < con->result.bits; i++)
	      constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;
            if (arg2)
              cnf_gen_uint_divmod (cnf, true, constresult, *arg1, *arg2);
            else if (arg2val != 0)
            { /* out=a/b, out and b is known, then out*b<=a<out*b+b */
              guint64 alo, ahi;
              alo = multiply64 (resultval, arg2val, ahi);
              if (((alo & ~mask) == 0 && ahi == 0)) /* if higher part is zero */
              {
                guint64 amin = alo;
                guint64 amax = alo+arg2val;
                if (amin + 1 != amax)
                {
		  cnf_gen_less (cnf, 0, *arg1, amin, DEF_NEGATIVE);
		  if (((amax)&mask) >= (amin&mask)) /* if not carry */
		    cnf_gen_less (cnf, 0, *arg1, amax, DEF_POSITIVE);
                }
                else /* amin+1 == amax */
                  cnf_gen_equal (cnf, 0, *arg1, amin, DEF_POSITIVE);
              }
              else
                cnf.add_clause (0);
            }
            else
              cnf.add_clause (0);
          }
          else
          {
            if (arg2)
            { /* out=a/b, out and a is known, a/(out+1)<b<=a/out */
              //cnf_gen_equal (cnf, 0, *arg2, 0, DEF_NEGATIVE);
              if (resultval != 0)
              {
                guint64 bmin = (resultval+1 != 0) ? arg1val/(resultval+1) : 0;
                guint64 bmax = arg1val/resultval;
                //bmin = std::max (1ULL, bmin);
                if (bmin < bmax)
                {
                  if (bmin+1 != bmax)
                  {
		    cnf_gen_lesseq (cnf, 0, *arg2, bmax, DEF_POSITIVE);
		    cnf_gen_lesseq (cnf, 0, *arg2, bmin, DEF_NEGATIVE);
		  }
		  else
		    cnf_gen_equal (cnf, 0, *arg2, bmax, DEF_POSITIVE);
		}
		else
		  cnf.add_clause (0);
              }
              else /* b>a */
              {
                cnf_gen_equal (cnf, 0, *arg2, 0, DEF_NEGATIVE);
                cnf_gen_lesseq (cnf, 0, *arg2, arg1val, DEF_NEGATIVE);
              }
            }
            else
            {
              if (arg2val == 0 || resultval != (arg1val/arg2val))
                cnf.add_clause (0);
            }
          }
        }
        break;
      case SPEAROP_SDIV:
        {
          guint64 signs = ~mask;
          guint64 signmask = 1ULL<<(con->result.bits-1);
          gint64 s_arg1val = arg1val;
          gint64 s_arg2val = arg2val;
          gint64 s_resultval = resultval;
          s_arg1val |= (arg1val&signmask) ? signs : 0;
          s_arg2val |= (arg2val&signmask) ? signs : 0;
          s_resultval |= (resultval&signmask) ? signs : 0;
	  if (result)
	  {
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_int_divmod (cnf, true, *result, *arg1, *arg2);
	      else if (s_arg2val != 0)
	        cnf_gen_int_divmod (cnf, true, *result, *arg1, s_arg2val);
	      else
	        cnf.add_clause (0);
	    }
	    else
	    {
	      if (arg2)
	      {
		LiteralVector constarg1 (con->args[0].bits);
		for (guint i = 0; i < con->args[0].bits; i++)
		  constarg1[i] = (arg1val&(1ULL<<i)) ? -zerovar : zerovar;

		cnf_gen_int_divmod (cnf, true, *result, constarg1, *arg2);
              }
              else if (s_arg2val != 0)
              {
                gint64 exp_result = s_arg1val/s_arg2val;
                guint64 required_sign = (arg1val&signmask) ^ (arg2val&signmask);
                if (required_sign == 0 && guint64 (exp_result) == signmask)
                  cnf.add_clause (0);
                else
                  cnf_gen_equal (cnf, 0, *result, exp_result,
                      DEF_POSITIVE);
              }
              else
                cnf.add_clause (0);
	    }
	  }
	  else
	  {
	    LiteralVector constresult (con->result.bits);
	    for (guint i = 0; i < con->result.bits; i++)
	      constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;

	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_int_divmod (cnf, true, constresult, *arg1, *arg2);
	      else if (s_arg2val != 0)
	      { /* out=a/b, out and b is known, then out*b<=a<out*b+b */
		guint64 alo, ahi;
		/* absolute values */
		guint64 abs_arg2val = std::abs (s_arg2val);
		guint64 abs_resultval = std::abs (s_resultval);

		LiteralVector temparg1 (*arg1);
		temparg1[temparg1.size ()-1] = -temparg1[temparg1.size ()-1];

		if (s_resultval != 0)
		{
		  alo = multiply64 (abs_resultval, abs_arg2val, ahi);
		  /* maximum value of abs(arg1val) */
		  guint64 threshold = signmask-1;
		  /* if arg1val must be signed */
		  guint64 required_sign = ((arg2val & signmask) ^
		      (resultval & signmask));
		  if (required_sign != 0)
		    threshold++;

		  if ((alo <= threshold) && ahi == 0) /* if higher part is zero */
		  {
		    guint64 amin = alo;
		    guint64 amax = alo+abs_arg2val;
		    if (required_sign == 0)
		    { /* arg1val positive */
		      if (amin+1 != amax)
		      { /* abs(out*b)<=a */
			cnf_gen_less (cnf, 0, temparg1, amin^signmask,
			    DEF_NEGATIVE);
			if (amax <= threshold)
			{ /* a<out*b+b */
			  cnf_gen_less (cnf, 0, temparg1, amax^signmask,
			      DEF_POSITIVE);
			}
		      }
		      else
		        cnf_gen_equal (cnf, 0, *arg1, amin, DEF_POSITIVE);
		    }
		    else
		    { /* arg1val negative */
		      if (amin+1 != amax)
		      { /* -abs(out*b)>=a */
			cnf_gen_lesseq (cnf, 0, temparg1, (-amin)^signmask,
			    DEF_POSITIVE);
			if (amax <= threshold)
			{ /* -abs(out*b+b)>a */
			  cnf_gen_lesseq (cnf, 0, temparg1, (-amax)^signmask,
			      DEF_NEGATIVE);
			}
		      }
		      else
		        cnf_gen_equal (cnf, 0, *arg1, (-amin), DEF_POSITIVE);
		    }
		  }
		  else
		    cnf.add_clause (0);
		}
		else
		{
		  /* -abs(arg2)<arg1 */
		  cnf_gen_lesseq (cnf, 0, temparg1, (-abs_arg2val) ^ signmask,
		      DEF_NEGATIVE);
		  /* arg1<abs(arg2) */
		  if (abs_arg2val < (1ULL<<(con->result.bits-1)))
		    cnf_gen_less (cnf, 0, temparg1, abs_arg2val ^ signmask,
		        DEF_POSITIVE);
		}
              }
	      else
	        cnf.add_clause (0);
	    }
	    else
	    {
	      if (arg2)
	      { /* out=a/b, out and a is known, a/(out+1)<b<=a/out */
	        /*LiteralVector constarg1 (con->args[0].bits);
		for (guint i = 0; i < con->args[0].bits; i++)
		  constarg1[i] = (arg1val & (1ULL<<i)) ? -zerovar : zerovar;
		cnf_gen_int_divmod (cnf, true, constresult, constarg1, *arg2);*/
		guint64 abs_arg1val = std::abs (s_arg1val);
		guint64 abs_resultval = std::abs (s_resultval);

		LiteralVector temparg2 (*arg2);
		temparg2[temparg2.size ()-1] = -temparg2[temparg2.size ()-1];

		if (abs_resultval != 0)
		{
		  guint64 bmin = (abs_resultval+1 != 0) ?
		      abs_arg1val/(abs_resultval+1) : 0;
		  guint64 bmax = abs_arg1val/abs_resultval;
		  //bmin = std::max (1ULL, bmin);
		  guint64 required_sign = ((arg1val & signmask) ^
		      (resultval & signmask));

		  if (bmin < bmax)
		  {
		    if (required_sign == 0)
		    {
		      if (bmax == signmask)
		        bmax--;
		      if (bmin+1 != bmax)
		      {
			cnf_gen_lesseq (cnf, 0, temparg2, bmax^signmask,
			     DEF_POSITIVE);
			cnf_gen_lesseq (cnf, 0, temparg2, bmin^signmask,
			     DEF_NEGATIVE);
		      }
		      else
			cnf_gen_equal (cnf, 0, *arg2, bmax, DEF_POSITIVE);
		    }
		    else
		    {
		      if (bmin+1 != bmax)
		      {
			cnf_gen_less (cnf, 0, temparg2, (-bmax)^signmask,
			     DEF_NEGATIVE);
			cnf_gen_less (cnf, 0, temparg2, (-bmin)^signmask,
			     DEF_POSITIVE);
		      }
		      else
			cnf_gen_equal (cnf, 0, *arg2, -bmax, DEF_POSITIVE);
		    }
		  }
		  else
		    cnf.add_clause (0);
		}
		else
		{ /* (abs(a)>b)|(-abs(a)<b) */
		  if (abs_arg1val != signmask)
		  {
		    cnf_gen_equal (cnf, 0, *arg2, 0, DEF_NEGATIVE);
		    //cnf_gen_less (cnf, 0, *arg2, resultval, DEF_NEGATIVE);
		    if (con->result.bits != 1)
		    {
		      gint32 comp1 = cnf.add_var ();
		      gint32 comp2 = cnf.add_var ();
		      cnf_gen_lesseq (cnf, -comp1, temparg2,
			  abs_arg1val^signmask, DEF_NEGATIVE);
		      cnf_gen_less (cnf, comp2, temparg2,
			  (-abs_arg1val)^signmask, DEF_POSITIVE);
		      cnf.add_clause (comp1, comp2);
		    }
		    else
		    {
		      if (arg1val == 0 && resultval == 0)
			cnf.add_clause ((*arg2)[0]);
		      else
			cnf.add_clause (0);
		    }
		  }
		  else /* abs(a)>b (unsatisfiable) */
		    cnf.add_clause (0);
		}
	      }
	      else if (s_arg2val == 0 || s_resultval != s_arg1val/s_arg2val)
		cnf.add_clause (0);
	    }
	  }
        }
        break;
      case SPEAROP_UMOD:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_uint_divmod (cnf, false, *result, *arg1, *arg2);
            else if (arg2val != 0)
            {
              guint bbits_n = 0;
              for (guint64 v = arg2val; v; v >>= 1, bbits_n++);

              if (bbits_n == con->result.bits)
                cnf_gen_uint_divmod (cnf, false, *result, *arg1, arg2val);
              else
              {
                LiteralVector tmpresult0 (result->begin (),
                    result->begin () + bbits_n);
                LiteralVector tmpresult1 (result->begin () + bbits_n,
                    result->end ());
                cnf_gen_uint_divmod (cnf, false, tmpresult0, *arg1, arg2val);
                cnf_gen_equal (cnf, 0, tmpresult1, 0, DEF_POSITIVE);
              }
            }
            else
              cnf.add_clause (0);
          }
          else
          {
            if (arg2)
            {
              LiteralVector constarg1 (con->args[0].bits);
              for (guint i = 0; i < con->args[0].bits; i++)
                constarg1[i] = (arg1val&(1ULL<<i)) ? -zerovar : zerovar;
              cnf_gen_uint_divmod (cnf, false, *result, constarg1, *arg2);
            }
            else if (arg2val != 0)
              cnf_gen_equal (cnf, 0, *result, arg1val%arg2val, DEF_POSITIVE);
            else
              cnf.add_clause (0);
          }
        }
        else
        {
          LiteralVector constresult (con->result.bits);
	  for (guint i = 0; i < con->result.bits; i++)
	    constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;
          if (arg1)
          {
            if (arg2)
              cnf_gen_uint_divmod (cnf, false, constresult, *arg1, *arg2);
            else if (arg2val != 0)
            {
              guint bbits_n = 0;
              for (guint64 v = arg2val; v; v >>= 1, bbits_n++);

              if (bbits_n == con->result.bits)
                cnf_gen_uint_divmod (cnf, false, constresult, *arg1, arg2val);
              else
              {
                if ((resultval & (~((1ULL<<bbits_n)-1))&mask) == 0)
                {
		  LiteralVector tmpresult0 (constresult.begin (),
		      constresult.begin () + bbits_n);
		  LiteralVector tmpresult1 (constresult.begin () + bbits_n,
		      constresult.end ());
		  cnf_gen_uint_divmod (cnf, false, tmpresult0, *arg1, arg2val);
		  //cnf_gen_equal (cnf, 0, tmpresult1, 0, DEF_POSITIVE);
                }
                else
                  cnf.add_clause (0);
              }
            }
            else
              cnf.add_clause (0);
          }
          else
          {
            if (arg2)
            { /* out=a%b, out and a is known, b= */
              LiteralVector constarg1 (con->args[0].bits);
	      for (guint i = 0; i < con->args[0].bits; i++)
		constarg1[i] = (arg1val & (1ULL<<i)) ? -zerovar : zerovar;
              cnf_gen_uint_divmod (cnf, false, constresult, constarg1, *arg2);
            }
            else
            {
              if (arg2val == 0 || resultval != (arg1val%arg2val))
                cnf.add_clause (0);
            }
          }
        }
        break;
      case SPEAROP_SMOD:
        {
          guint64 signs = ~mask;
          guint64 signmask = 1ULL<<(con->result.bits-1);
          gint64 s_arg1val = arg1val;
          gint64 s_arg2val = arg2val;
          gint64 s_resultval = resultval;
          s_arg1val |= (arg1val&signmask) ? signs : 0;
          s_arg2val |= (arg2val&signmask) ? signs : 0;
          s_resultval |= (resultval&signmask) ? signs : 0;
	  if (result)
	  {
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_int_divmod (cnf, false, *result, *arg1, *arg2);
	      else if (s_arg2val != 0)
	      {
		guint bbits_n = 0;
		if (gint64 (s_arg2val) >= 0)
		  for (guint64 v = arg2val; v; v >>= 1, bbits_n++);
		else
		  for (gint64 v = s_arg2val; v != -1LL; v >>= 1, bbits_n++);
		bbits_n++;

		if (bbits_n == con->result.bits)
		  cnf_gen_int_divmod (cnf, false, *result, *arg1, s_arg2val);
		else
		{
		  LiteralVector tmpresult0 (result->begin (),
		      result->begin () + bbits_n);
		  LiteralVector tmpresult1 (result->begin () + bbits_n,
		      result->end ());
		  cnf_gen_int_divmod (cnf, false, tmpresult0, *arg1, s_arg2val);

		  LiteralVector resultsigns (con->result.bits-bbits_n,
		      (*result)[bbits_n-1]);
		  cnf_gen_equal (cnf, 0, tmpresult1, resultsigns, DEF_POSITIVE);
		}
	      }
	      else
	        cnf.add_clause (0);
	    }
	    else
	    {
	      if (arg2)
	      {
		LiteralVector constarg1 (con->args[0].bits);
		for (guint i = 0; i < con->args[0].bits; i++)
		  constarg1[i] = (arg1val&(1ULL<<i)) ? -zerovar : zerovar;
		cnf_gen_int_divmod (cnf, false, *result, constarg1, *arg2);
	      }
	      else if (arg2val != 0)
	      {
	        guint64 out = guint64 (std::abs (s_arg1val)) %
	           guint64 (std::abs (s_arg2val));
	        out = (s_arg1val < 0) ? -out : out;
		cnf_gen_equal (cnf, 0, *result, out, DEF_POSITIVE);
              }
	      else
		cnf.add_clause (0);
	    }
	  }
	  else
	  {
	    LiteralVector constresult (con->result.bits);
	    for (guint i = 0; i < con->result.bits; i++)
	      constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;
	    if (arg1)
	    {
	      if (arg2)
		cnf_gen_int_divmod (cnf, false, constresult, *arg1, *arg2);
	      else if (s_arg2val != 0)
	      {
		guint bbits_n = 0;
		if (gint64 (s_arg2val) >= 0)
		  for (guint64 v = arg2val; v; v >>= 1, bbits_n++);
		else
		  for (gint64 v = s_arg2val; v != -1LL; v >>= 1, bbits_n++);
		bbits_n++;

		if (bbits_n == con->result.bits)
		  cnf_gen_int_divmod (cnf, false, constresult, *arg1, s_arg2val);
		else
		{
		  guint64 newsignmask = 1ULL<<(bbits_n-1);
		  guint64 highermask = ~lomask (bbits_n);
		  guint64 signs = (resultval&newsignmask) ? highermask&mask : 0;
		  if ((s_resultval & (highermask&mask)) == signs)
		  {
		    LiteralVector tmpresult0 (constresult.begin (),
			constresult.begin () + bbits_n);
		    LiteralVector tmpresult1 (constresult.begin () + bbits_n,
			constresult.end ());
		    cnf_gen_int_divmod (cnf, false, tmpresult0, *arg1, s_arg2val);
		  }
		  else
		    cnf.add_clause (0);
		}
	      }
	      else
	        cnf.add_clause (0);
	    }
	    else
	    {
	      if (arg2)
	      { /* out=a%b, out and a is known, b= */
		LiteralVector constarg1 (con->args[0].bits);
		for (guint i = 0; i < con->args[0].bits; i++)
		  constarg1[i] = (arg1val & (1ULL<<i)) ? -zerovar : zerovar;
		cnf_gen_int_divmod (cnf, false, constresult, constarg1, *arg2);
	      }
	      else if (s_arg2val != 0)
	      {
	        guint64 out = guint64 (std::abs (s_arg1val)) %
	           guint64 (std::abs (s_arg2val));
	        out = (s_arg1val < 0) ? -out : out;
		if (s_resultval != gint64 (out))
		  cnf.add_clause (0);
	      }
	      else
	        cnf.add_clause (0);
	    }
	  }
	}
        break;
      /* shifts is compliant with book specification
         (Spear Modular Arithmetic Format Specification 1.0 (Rev.: 1.7) */
      case SPEAROP_SL:
        if (result)
        {
          if (arg1)
          {
            if (arg2)
              cnf_gen_shl (cnf, *result, *arg1, *arg2, false);
            else
            {
              guint shift = std::min (guint (arg2val), con->args[0].bits);
              LiteralVector result0 (result->begin (), result->begin () + shift);
              LiteralVector result1 (result->begin () + shift, result->end ());

              if (result0.size ())
                cnf_gen_equal (cnf, 0, result0, 0, DEF_POSITIVE);
              if (result1.size ())
              {
                LiteralVector temparg (arg1->begin (), arg1->end () - shift);
                cnf_gen_equal (cnf, 0, result1, temparg, DEF_POSITIVE);
              }
            }
          }
          else
          {
            if (arg2)
              cnf_gen_shl (cnf, *result, arg1val, *arg2, false);
            else
            {
              guint shift = std::min (guint (arg2val), con->args[0].bits);
              LiteralVector result0 (result->begin (), result->begin () + shift);
              LiteralVector result1 (result->begin () + shift, result->end ());

              if (result0.size ())
                cnf_gen_equal (cnf, 0, result0, 0, DEF_POSITIVE);
              if (result1.size ())
              {
                guint64 shiftmask = lomask (con->args[0].bits-shift);
                cnf_gen_equal (cnf, 0, result1, arg1val & shiftmask, DEF_POSITIVE);
              }
            }
          }
        }
        else
        {
          if (arg1)
          {
            if (arg2)
            {
	      LiteralVector constresult (con->result.bits);
	      for (guint i = 0; i < con->result.bits; i++)
		constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;

	      cnf_gen_shl (cnf, constresult, *arg1, *arg2, false);
            }
            else
            {
              guint shift = std::min (guint (arg2val), con->args[0].bits);
              if ((resultval & lomask (shift)) == 0)
              {
                if (con->result.bits > shift)
                {
		  LiteralVector temparg (arg1->begin (), arg1->end () - shift);
		  cnf_gen_equal (cnf, 0, temparg, shr64 (resultval, shift),
		        DEF_POSITIVE);
                }
              }
              else /* if not satisfiable */
                cnf.add_clause (0);
            }
          }
          else
          {
            if (arg2)
            {
              LiteralVector constresult (con->result.bits);
	      for (guint i = 0; i < con->result.bits; i++)
		constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;

	      cnf_gen_shl (cnf, constresult, arg1val, *arg2, false);
            }
            else
            {
              guint shift = std::min (guint (arg2val), con->args[0].bits);
              if (resultval != (shl64 (arg1val, shift)&mask))
                cnf.add_clause (0);
            }
          }
        }
        break;
      case SPEAROP_ASR:
      case SPEAROP_LSR:
        {
          bool with_sign = (con->op == SPEAROP_ASR);
	  if (result)
	  {
	    if (arg1)
	    {
	      if (arg2)
	        cnf_gen_shr (cnf, *result, *arg1, *arg2, with_sign);
	      else
	      {
	        guint shift = std::min (guint (arg2val), con->args[0].bits);
	        LiteralVector result0 (result->begin (), result->end () - shift);
	        LiteralVector result1 (result->end () - shift, result->end ());

	        if (result1.size ())
	        {
		  if (with_sign)
		  {
		    LiteralVector signs (shift, (*arg1)[arg1->size ()-1]);
		    cnf_gen_equal (cnf, 0, result1, signs, DEF_POSITIVE);
		  }
		  else
		    cnf_gen_equal (cnf, 0, result1, 0, DEF_POSITIVE);
	        }
	        if (result0.size ())
	        {
	          LiteralVector temparg (arg1->begin () + shift, arg1->end ());
	          cnf_gen_equal (cnf, 0, result0, temparg, DEF_POSITIVE);
	        }
	      }
	    }
	    else
	    {
	      if (arg2)
	        cnf_gen_shr (cnf, *result, arg1val, con->args[0].bits,
	           *arg2, with_sign);
	      else
	      {
	        guint shift = std::min (guint (arg2val), con->args[0].bits);
	        LiteralVector result0 (result->begin (), result->end () - shift);
	        LiteralVector result1 (result->end () - shift, result->end ());

	        if (result1.size ())
	        { /* sign value used to compare with higher result */
	          guint64 signval = (with_sign &&
	             (arg1val & (1ULL<<(con->args[0].bits-1))) != 0) ?
	               G_MAXUINT64 : 0;
	          cnf_gen_equal (cnf, 0, result1, signval, DEF_POSITIVE);
	        }
	        if (result0.size ())
	        {
	          guint64 shiftmask = lomask (con->args[0].bits-shift);
	          cnf_gen_equal (cnf, 0, result0,
	             (shr64 (arg1val, shift))&shiftmask, DEF_POSITIVE);
	        }
	      }
	    }
	  }
	  else
	  {
	    if (arg1)
	    {
	      if (arg2)
	      {
	        LiteralVector constresult (con->result.bits);
		for (guint i = 0; i < con->result.bits; i++)
		  constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;

		cnf_gen_shr (cnf, constresult, *arg1, *arg2, with_sign);
	      }
	      else
	      {
	        guint shift = std::min (guint (arg2val), con->args[0].bits);
	        LiteralVector temparg (arg1->begin () + shift, arg1->end ());

	        guint64 shiftmask = lomask (shift);
	        /* revshiftmask - mask used for filtering bits of
	           resultval compare with higher arg1 bits */
	        guint64 revshiftmask = lomask (con->result.bits-shift);
	        if (temparg.size ())
	          cnf_gen_equal (cnf, 0, temparg, resultval&revshiftmask,
		      DEF_POSITIVE);

	        if (with_sign)
	        {
	          if (shift != 0)
	          {
		    LiteralVector signs (shift, (*arg1)[arg1->size ()-1]);
		    cnf_gen_equal (cnf, 0, signs,
		        shr64 (resultval, con->args[0].bits-shift), DEF_POSITIVE);
	          }
	        }
	        else if (shift != 0)
	        { /* if higher bits of resultval must be zero */
	          if ((shr64 (resultval, con->args[0].bits-shift)
	                 & shiftmask) != 0)
	            cnf.add_clause (0);
	        }
	      }
	    }
	    else
	    {
	      if (arg2)
	      {
	        LiteralVector constresult (con->result.bits);
		for (guint i = 0; i < con->result.bits; i++)
		  constresult[i] = (resultval & (1ULL<<i)) ? -zerovar : zerovar;

		cnf_gen_shr (cnf, constresult, arg1val, con->args[0].bits,
		      *arg2, with_sign);
	      }
	      else
	      {
                guint shift = std::min (guint (arg2val), con->args[0].bits);
                /* signbit - sign bit of arg1 */
                guint64 signbit = 1ULL<<(con->result.bits-1);
                /* signs - value fo signs or'ed with shifted arg1val */
                guint64 signs = (with_sign && (arg1val & signbit)) ?
                    ~lomask (con->args[0].bits-shift) : 0;
                if ((resultval&mask) != (((shr64 (arg1val, shift)|signs)&mask)))
                  cnf.add_clause (0);
	      }
	    }
	  }
        }
        break;
      case SPEAROP_TRUN:
        if (result)
        {
          if (arg1)
          {
            LiteralVector temparg (arg1->begin (),
                arg1->begin () + con->result.bits);
            cnf_gen_equal (cnf, 0, *result, temparg, DEF_POSITIVE);
          }
          else
            cnf_gen_equal (cnf, 0, *result, arg1val, DEF_POSITIVE);
        }
        else
        {
          if (arg1)
          {
            LiteralVector temparg (arg1->begin (),
                arg1->begin () + con->result.bits);
            cnf_gen_equal (cnf, 0, temparg, resultval, DEF_POSITIVE);
          }
          else if (resultval != (arg1val&mask))
            cnf.add_clause (0);
        }
        break;
      case SPEAROP_SEXT:
        {
          guint64 newarg1val = 0;
          guint64 amask = lomask (con->args[0].bits);
          if (!arg1)
          {
            guint64 asign = ((arg1val & (1ULL<<(con->args[0].bits-1))) != 0) ?
                ~amask : 0;
            newarg1val = (arg1val & amask) | asign;
	  }
	  if (result)
	  {
	    if (arg1)
	    {
	      LiteralVector temparg = *arg1;
	      temparg.insert (temparg.end (), con->result.bits-con->args[0].bits,
		  (*arg1)[arg1->size ()-1]);
	      cnf_gen_equal (cnf, 0, *result, temparg, DEF_POSITIVE);
	    }
	    else
	      cnf_gen_equal (cnf, 0, *result, newarg1val, DEF_POSITIVE);
	  }
	  else
	  {
	    if (arg1)
	    {
	      guint64 resultext = (resultval & (1ULL<<(con->args[0].bits-1))) ?
	           ~amask & mask : 0;
	      if (resultext == ((resultval & ~amask) & mask))
	        cnf_gen_equal (cnf, 0, *arg1, resultval, DEF_POSITIVE);
	      else
	       cnf.add_clause (0);
	    }
	    else if ((newarg1val&mask) != (resultval & mask))
	      cnf.add_clause (0);
	  }
        }
        break;
      case SPEAROP_ZEXT:
        if (result)
        {
          if (arg1)
          {
            LiteralVector result0 (result->begin (), result->begin () +
                con->args[0].bits);
            LiteralVector result1 (result->begin () + con->args[0].bits,
                result->end ());
            cnf_gen_equal (cnf, 0, result0, *arg1, DEF_POSITIVE);
            cnf_gen_equal (cnf, 0, result1, 0, DEF_POSITIVE);
          }
          else
            cnf_gen_equal (cnf, 0, *result, arg1val, DEF_POSITIVE);
        }
        else
        {
          guint64 amask =  lomask (con->args[0].bits);
          if (arg1)
          {
            if (((resultval & ~amask) & mask) != 0)
                /* higer part result must be zero (otherwise unsatisfiable) */
              cnf.add_clause (0);
            else
              cnf_gen_equal (cnf, 0, *arg1, resultval, DEF_POSITIVE);
          }
          else if ((arg1val&amask) != (resultval&mask))
            cnf.add_clause (0);
        }
        break;
      case SPEAROP_CONC:
        if (result)
        {
          LiteralVector result0 (result->begin (),
              result->begin () + con->args[1].bits);
          LiteralVector result1 (result->begin () + con->args[1].bits,
              result->end ());

          if (arg2)
            cnf_gen_equal (cnf, 0, result0, *arg2, DEF_POSITIVE);
          else
            cnf_gen_equal (cnf, 0, result0, arg2val, DEF_POSITIVE);

          if (arg1)
            cnf_gen_equal (cnf, 0, result1, *arg1, DEF_POSITIVE);
          else
            cnf_gen_equal (cnf, 0, result1, arg1val, DEF_POSITIVE);
        }
        else
        {
          guint64 arg1mask = lomask (con->args[0].bits);
          guint64 arg2mask = lomask (con->args[1].bits);
          guint64 result0val = resultval & arg2mask;
          guint64 result1val = (resultval >> con->args[1].bits) & arg1mask;

          if (arg2)
            cnf_gen_equal (cnf, 0, *arg2, result0val, DEF_POSITIVE);
          else if (result0val != (arg2val & arg2mask))
            cnf.add_clause (0);

          if (arg1)
            cnf_gen_equal (cnf, 0, *arg1, result1val, DEF_POSITIVE);
          else if (result1val != (arg1val & arg1mask))
            cnf.add_clause (0);
        }
        break;
      case SPEAROP_EXTR:
        if (result)
        {
          if (arg1)
          {
            LiteralVector newarg1 (arg1->begin () + arg2val,
                  arg1->begin () + arg3val);
            cnf_gen_equal (cnf, 0, *result, newarg1, DEF_POSITIVE);
          }
          else
          {
            guint64 newarg1val = shr64 (arg1val, arg2val) &
                lomask (arg3val-arg2val);
            /*guint64 newarg1val = (arg1val >> arg2val) &
                ((1ULL << (arg3val-arg2val))-1);*/
            cnf_gen_equal (cnf, 0, *result, newarg1val, DEF_POSITIVE);
          }
        }
        else
        {
          if (arg1)
          {
            LiteralVector newarg1 (arg1->begin () + arg2val,
                  arg1->begin () + arg3val);
            cnf_gen_equal (cnf, 0, newarg1, resultval, DEF_POSITIVE);
          }
          else
          {
            //guint64 mask = (1ULL << (con->result.bits))-1;
            /*guint64 newarg1val = (arg1val >> arg2val) &
                ((1ULL << (arg3val-arg2val))-1);*/
            guint64 newarg1val = shr64 (arg1val, arg2val) &
                lomask (arg3val-arg2val);
            if (resultval != newarg1val) /* if unsatisfiable */
              cnf.add_clause (0);
          }
        }
        break;
      default:
        break;
    }
  }
}

};

void
SpearFormatModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  gint32 zerovar = cnf.add_var ();
  cnf.add_clause (-zerovar);

  guint vars_n = varids.size ();
  std::vector<LiteralVector> vars_literals (vars_n);
  for (SpearVarMap::const_iterator it = varids.begin (); it != varids.end (); ++it)
    vars_literals[it->second] =
        cnf.add_vars_with_literals (var_sizes[it->second]);

  if (with_outmap)
  {
    outmap_string = "s SATISFIABLE\n";
    for (SpearVarMap::const_iterator it = varids.begin ();
         it != varids.end (); ++it)
    {
      guint varid = it->second;
      std::ostringstream os;
      os << "v " << it->first << "={ u:";
      for (LiteralConstIter lit = vars_literals[varid].begin (); lit !=
           vars_literals[varid].end (); ++lit)
         os << ' ' << *lit;
      os << " }\n";
      outmap_string += os.str ();
    }
  }

  /* convert predicates to constraints */
  SpearConstraintVector constraints_from_preds;
  for (SpearPredicateConstIter pred = predicates.begin ();
       pred != predicates.end (); ++pred)
  {
    SpearConstraint constraint;
    constraint.result.bits = 1;
    constraint.result.value = 1;
    constraint.result.constant = true;
    constraint.op = pred->pred;
    for (guint i = 0; i < 3; i++)
      constraint.args[i] = pred->args[i];
    constraints_from_preds.push_back (constraint);
  }

  generate_from_constraints (cnf, constraints, vars_literals, zerovar);
  generate_from_constraints (cnf, constraints_from_preds, vars_literals, zerovar);
}
