/*
 * genarith-test.cc - genarith test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

static bool
check_cnf_satisfability (const CNF& cnf, const LiteralVector& vars,
    guint64 val)
{
  LiteralVector others;
  std::vector<bool> inputs (cnf.get_vars_n ());
  
  /* put others vars to others vector */
  for (gint i = 1; i <= cnf.get_vars_n (); i++)
    if (std::find (vars.begin (), vars.end (), i) == vars.end ())
      others.push_back (i);
  
  for (guint i = 0; i < vars.size (); i++)
    inputs[vars[i]-1] = ((val & (1ULL << i)) == (1ULL << i));
  
  if (others.size ())
  {
    guint64 comb_num = 1ULL << others.size ();
    for (guint64 v = 0; v < comb_num; v++)
    {
      for (guint j = 0; j < others.size (); j++)
	inputs[others[j]-1] = ((v & (1ULL << j)) == (1ULL << j));
      if (cnf.evaluate (inputs))
	return true;
    }
  }
  else if (cnf.evaluate (inputs))
    return true;
  
  return false;
}

static bool
gen_equal_test ()
{
  CNF cnf;
  LiteralVector a;
  gint32 r;
  
  std::cout << "Testing generating of var==const (positive)" << std::endl;
  a = cnf.add_vars_with_literals (5);
  cnf_gen_equal (cnf, 0, a, 0x19, DEF_POSITIVE);
  for (guint i = 0; i < 32; i++)
    if (check_cnf_satisfability (cnf, a, i) != (i == 0x19))
      return false;
  
  std::cout << "Testing generating of var==const (negative)" << std::endl;
  cnf.clear ();
  a = cnf.add_vars_with_literals (5);
  cnf_gen_equal (cnf, 0, a, 0x19, DEF_NEGATIVE);
  for (guint i = 0; i < 32; i++)
    if (check_cnf_satisfability (cnf, a, i) != (i != 0x19))
      return false;
  
  std::cout << "Testing generating of result=>(var==const) (positive)" << std::endl;
  cnf.clear ();
  r = cnf.add_var ();
  a = cnf.add_vars_with_literals (5);
  cnf_gen_equal (cnf, r, a, 0x19, DEF_POSITIVE);
  cnf.add_clause (r);
  for (guint i = 0; i < 32; i++)
    if (check_cnf_satisfability (cnf, a, i) != (i == 0x19))
      return false;
  
  std::cout << "Testing generating of -result<=(var==const) (negative)" << std::endl;
  cnf.clear ();
  r = cnf.add_var ();
  a = cnf.add_vars_with_literals (5);
  cnf_gen_equal (cnf, -r, a, 0x19, DEF_NEGATIVE);
  cnf.add_clause (r);
  for (guint i = 0; i < 32; i++)
    if (check_cnf_satisfability (cnf, a, i) != (i != 0x19))
      return false;
  
  return true;
}

struct TestSuiteSample
{
  guint64 a, b;
};

struct TestSuite
{
  guint abits_n, bbits_n;
  guint samples_n;
  const TestSuiteSample* samples;
};

static bool
gen_uint_less_test ()
{
  CNF cnf;
  LiteralVector a;
  gint32 r;
  
  /*{
    CNF cnf2;
    LiteralVector a = cnf2.add_vars_with_literals (64);
    guint64 bval = 0x5488a2be9a9a1237;
    //guint64 bval = 0x084ab7500e8d3a0;
    //guint64 bval = 0x080888800000000;
    gint32 r1 = cnf2.add_var ();
    gint32 r2 = cnf2.add_var ();
    cnf_gen_less (cnf2, r1, a, bval, DEF_FULL);
    LiteralVector b = cnf2.add_vars_with_literals (64);
    cnf_gen_equal (cnf2, 0, b, bval, DEF_POSITIVE);
    cnf_gen_less (cnf2, r2, a, b, DEF_FULL);
    LiteralVector r1v, r2v;
    r1v.push_back (r1);
    r2v.push_back (r2);
    
    cnf_gen_equal (cnf2, 0, r1v, r2v, DEF_NEGATIVE);
    cnf2.save_to_file ("less-test.cnf");
  }*/
  
  for (guint bits = 1; bits <= 6; bits++)
  {
    //guint bits = 5;
    std::cout << "Less test bits: " << bits << std::endl;
    for (guint64 b = 0; b < (1ULL << bits); b++)
    {
      //r = 0;
      std::cout << "For value: " << b << std::endl;
      std::cout <<
        "  Testing generating of result=>(var<const) (positive)" << std::endl;
      cnf.clear ();
      r = cnf.add_var ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_less (cnf, r, a, b, DEF_POSITIVE);
      cnf.add_clause (r);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i < b))
	  return false;
      
      std::cout <<
        "  Testing generating of -result<=(var<const) (negative)" << std::endl;
      cnf.clear ();
      r = cnf.add_var ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_less (cnf, -r, a, b, DEF_NEGATIVE);
      cnf.add_clause (r);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i >= b))
	  return false;
      
      std::cout <<
        "  Testing generating of result=>(var<=const) (positive)" << std::endl;
      cnf.clear ();
      r = cnf.add_var ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_lesseq (cnf, r, a, b, DEF_POSITIVE);
      cnf.add_clause (r);
      //cnf.save_to_file ("out.cnf");
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i <= b))
	  return false;
      
      std::cout <<
        "  Testing generating of -result<=(var<=const) (negative)" << std::endl;
      cnf.clear ();
      r = cnf.add_var ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_lesseq (cnf, -r, a, b, DEF_NEGATIVE);
      cnf.add_clause (r);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i > b))
	  return false;

      
      std::cout << "For value: " << b << std::endl;
      std::cout <<
        "  Testing generating of (var<const) (positive)" << std::endl;
      cnf.clear ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_less (cnf, 0, a, b, DEF_POSITIVE);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i < b))
	  return false;
      
      std::cout <<
        "  Testing generating of (var<const) (negative)" << std::endl;
      cnf.clear ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_less (cnf, 0, a, b, DEF_NEGATIVE);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i >= b))
	  return false;
      
      std::cout <<
        "  Testing generating of (var<=const) (positive)" << std::endl;
      cnf.clear ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_lesseq (cnf, 0, a, b, DEF_POSITIVE);
      //cnf.save_to_file ("out.cnf");
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i <= b))
	  return false;
      
      std::cout <<
        "  Testing generating of (var<=const) (negative)" << std::endl;
      cnf.clear ();
      a = cnf.add_vars_with_literals (bits);
      cnf_gen_lesseq (cnf, 0, a, b, DEF_NEGATIVE);
      for (guint i = 0; i < (1U << bits); i++)
	if (check_cnf_satisfability (cnf, a, i) != (i > b))
	  return false;
    }
  }
  
  /* 1bit, 1bit */
  static const TestSuiteSample var_var_tests_1_1[] =
  {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
  };
  static const TestSuiteSample var_var_tests_2_2[] =
  {
    { 0, 1 }, { 1, 2 }, { 1, 3 }, { 2, 3 },
    { 3, 1 }, { 2, 2 }, { 0, 0 }, { 2, 1 },
  };
  static const TestSuiteSample var_var_tests_3_3[] =
  {
    { 3, 7 }, { 2, 1 }, { 5, 2 }, { 6, 7 },
    { 5, 5 }, { 5, 1 }, { 4, 2 }, { 7, 5 },
    { 1, 2 }, { 2, 2 }, { 7, 7 }, { 3, 5 },
  };
  static const TestSuiteSample var_var_tests_4_4[] =
  {
    { 11, 7 }, { 12, 3 }, { 8, 9 }, { 2, 14 },
    { 15, 5 }, { 2, 10 }, { 13, 5 }, { 4, 7 },
    { 13, 13 }, { 15, 0 }, { 0, 0 }, { 14, 15 },
  };
  static const TestSuiteSample var_var_tests_5_5[] =
  {
    { 24, 23 }, { 11, 27}, { 30, 21 }, { 12, 17 },
    { 1, 8 }, { 11, 16 }, { 19, 25 }, { 24, 15 },
    { 14, 27 }, { 30, 31 }, { 0, 0 }, { 30, 0 },
    { 21, 31 }, { 1, 31 }, { 5, 28 }, { 13, 7 },
  };
  static const TestSuiteSample var_var_tests_6_6[] =
  {
    { 24, 43 }, { 43, 43 }, { 21, 59 }, { 3, 63 },
    { 37, 36 }, { 46, 47 }, { 33, 52 }, { 56, 10 },
  };
  /*static const TestSuiteSample var_var_tests_3_4[] =
  {
    { 3, 8 }, { 7, 5 }, { 5, 2 }, { 6, 11 },
    { 5, 5 }, { 5, 15 }, { 14, 2 }, { 7, 5 },
    { 0, 0 }, { 6, 6 }
  };
  static const TestSuiteSample var_var_tests_4_3[] =
  {
    { 8, 3 }, { 5, 7 }, { 2, 5 }, { 11, 6 },
    { 5, 5 }, { 15, 5 }, { 2, 4 }, { 5, 7 },
    { 0, 0 }, { 6, 6 }
  };*/
  
  static const TestSuite var_var_testsuite[] =
  {
    {
      1, 1, 4,
      var_var_tests_1_1
    },
    {
      2, 2, 8,
      var_var_tests_2_2
    },
    {
      3, 3, 12,
      var_var_tests_3_3
    },
    {
      4, 4, 12,
      var_var_tests_4_4
    },
    {
      5, 5, 16,
      var_var_tests_5_5
    },
    {
      6, 6, 8,
      var_var_tests_6_6
    },
    /*{
      3, 4, 10,
      var_var_tests_3_4,
    },
    {
      4, 3, 10,
      var_var_tests_4_3,
    },*/
  };
  static const gsize var_var_testsuites_n =
        sizeof(var_var_testsuite)/sizeof (TestSuite);
  
  LiteralVector b;
  
  for (guint i = 0; i < var_var_testsuites_n; i++)
  {
    guint asize = var_var_testsuite[i].abits_n;
    guint bsize = var_var_testsuite[i].bbits_n;
    
    std::cout << "ASize: " << asize << ", BSize: " << bsize << std::endl;
    
    LiteralVector all;
    const TestSuiteSample* samples = var_var_testsuite[i].samples;

    std::cout << "  Testing generating of r=>(var1<var2) (positive)" << std::endl;
    cnf.clear ();
    r = cnf.add_var ();
    a = cnf.add_vars_with_literals (asize);
    b = cnf.add_vars_with_literals (bsize);
    cnf.add_clause (r);
    cnf_gen_less (cnf, r, a, b, DEF_POSITIVE);
    
    all = a;
    all.insert (all.end (), b.begin (), b.end ());
    for (guint j = 0; j < var_var_testsuite[i].samples_n; j++)
    {
      if (check_cnf_satisfability (cnf, all,
              samples[j].a + (samples[j].b<<(asize))) !=
                    (samples[j].a < samples[j].b))
	return false;
    }

    std::cout << "  Testing generating of -r<=(var1<var2) (negative)" << std::endl;
    cnf.clear ();
    r = cnf.add_var ();
    a = cnf.add_vars_with_literals (asize);
    b = cnf.add_vars_with_literals (bsize);
    cnf.add_clause (r);
    cnf_gen_less (cnf, -r, a, b, DEF_NEGATIVE);
    
    all = a;
    all.insert (all.end (), b.begin (), b.end ());
    for (guint j = 0; j < var_var_testsuite[i].samples_n; j++)
      if (check_cnf_satisfability (cnf, all,
              samples[j].a + (samples[j].b<<(asize))) !=
                    (samples[j].a >= samples[j].b))
	return false;
  
    std::cout << "  Testing generating of r=>(var1<=var2) (positive)" << std::endl;
    cnf.clear ();
    r = cnf.add_var ();
    a = cnf.add_vars_with_literals (asize);
    b = cnf.add_vars_with_literals (bsize);
    cnf.add_clause (r);
    cnf_gen_lesseq (cnf, r, a, b, DEF_POSITIVE);
    
    all = a;
    all.insert (all.end (), b.begin (), b.end ());
    for (guint j = 0; j < var_var_testsuite[i].samples_n; j++)
    {
      if (check_cnf_satisfability (cnf, all,
              samples[j].a + (samples[j].b<<(asize))) !=
                    (samples[j].a <= samples[j].b))
	return false;
    }

    std::cout << "  Testing generating of -r<=(var1<=var2) (negative)" << std::endl;
    cnf.clear ();
    r = cnf.add_var ();
    a = cnf.add_vars_with_literals (asize);
    b = cnf.add_vars_with_literals (bsize);
    cnf.add_clause (r);
    cnf_gen_lesseq (cnf, -r, a, b, DEF_NEGATIVE);
    
    all = a;
    all.insert (all.end (), b.begin (), b.end ());
    for (guint j = 0; j < var_var_testsuite[i].samples_n; j++)
      if (check_cnf_satisfability (cnf, all,
              samples[j].a + (samples[j].b<<(asize))) !=
                    (samples[j].a > samples[j].b))
	return false;
  }
  
  return true;
}
#if 0
static bool
gen_add_test ()
{

  CNF cnf;
  LiteralVector a, b, c;
//#if 0
  std::cout << "Testing adder for 4 bits without out carry" << std::endl;
  for (guint c0v = 0; c0v < 2; c0v++)
    for (guint av = 0; av < 16; av++)
      for (guint bv = 0; bv < 16; bv++)
      {
	std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	{
	  cnf.clear ();
	  a = cnf.add_vars_with_literals (4);
	  b = cnf.add_vars_with_literals (4);
	  c = cnf.add_vars_with_literals (5);
	  gint32 c0 = cnf.add_var ();
	  LiteralVector c0vec;
	  c0vec.push_back (c0);
	  cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, b, bv, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	  cnf_gen_add (cnf, c, a, b, c0, false);
	  //cnf_gen_add (cnf, c, a, b, 0, false);
	  
	  for (guint cv = 0; cv < 32; cv++)
	    if (check_cnf_satisfability (cnf, c, cv) !=
		(((av+bv+c0v)&0x1f) == cv))
	    return false;
        }
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
        {
	  cnf.clear ();
	  a = cnf.add_vars_with_literals (4);
	  b = cnf.add_vars_with_literals (4);
	  c = cnf.add_vars_with_literals (4);
	  gint32 c0 = cnf.add_var ();
	  LiteralVector c0vec;
	  c0vec.push_back (c0);
	  cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, b, bv, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	  cnf_gen_add (cnf, c, a, b, c0, false);
	  //cnf_gen_add (cnf, c, a, b, 0, false);
	  
	  for (guint cv = 0; cv < 16; cv++)
	    if (check_cnf_satisfability (cnf, c, cv) !=
		(((av+bv+c0v)&0xf) == cv))
	    return false;
        }
        
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	cnf.clear ();
	a = cnf.add_vars_with_literals (4);
	b = cnf.add_vars_with_literals (4);
	c = cnf.add_vars_with_literals (5);
	/*gint32 c0 = cnf.add_var ();
	LiteralVector c0vec;
	c0vec.push_back (c0);*/
	cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	cnf_gen_equal (cnf, 0, b, bv, DEF_POSITIVE);
	//cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	//cnf_gen_add (cnf, c, a, b, c0, false);
	cnf_gen_add (cnf, c, a, b, 0, c0v);
	
	for (guint cv = 0; cv < 32; cv++)
	  if (check_cnf_satisfability (cnf, c, cv) !=
	      (((av+bv+c0v)&0x1f) == cv))
	  return false;
        
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	cnf.clear ();
	a = cnf.add_vars_with_literals (4);
	b = cnf.add_vars_with_literals (4);
	c = cnf.add_vars_with_literals (4);
	/*gint32 c0 = cnf.add_var ();
	LiteralVector c0vec;
	c0vec.push_back (c0);*/
	cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	cnf_gen_equal (cnf, 0, b, bv, DEF_POSITIVE);
	//cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	//cnf_gen_add (cnf, c, a, b, c0, false);
	cnf_gen_add (cnf, c, a, b, 0, c0v);
	
	for (guint cv = 0; cv < 16; cv++)
	  if (check_cnf_satisfability (cnf, c, cv) !=
	      (((av+bv+c0v)&0xf) == cv))
	  return false;
      }
//#endif
  //LiteralVector a, c;
  std::cout << "Testing adder with constant input" << std::endl;
  
  for (guint c0v = 0; c0v < 2; c0v++)
    for (guint av = 0; av < 32; av++)
      for (guint64 bv = 0; bv < 32; bv++)
      {
	std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	{
	  cnf.clear ();
	  a = cnf.add_vars_with_literals (5);
	  c = cnf.add_vars_with_literals (6);
	  gint32 c0 = cnf.add_var ();
	  LiteralVector c0vec;
	  c0vec.push_back (c0);
	  cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	  cnf_gen_add (cnf, c, a, bv, c0, true);
	  
	  for (guint cv = 0; cv < 64; cv++)
	    if (check_cnf_satisfability (cnf, c, cv) !=
		  (((av+bv+c0v)&0x3f) == cv))
	      return false;
        }
        
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	{
	  cnf.clear ();
	  a = cnf.add_vars_with_literals (5);
	  c = cnf.add_vars_with_literals (5);
	  gint32 c0 = cnf.add_var ();
	  LiteralVector c0vec;
	  c0vec.push_back (c0);
	  cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	  cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	  cnf_gen_add (cnf, c, a, bv, c0, true);
	  
	  for (guint cv = 0; cv < 32; cv++)
	    if (check_cnf_satisfability (cnf, c, cv) !=
		  (((av+bv+c0v)&0x1f) == cv))
	      return false;
	}
	
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	cnf.clear ();
	a = cnf.add_vars_with_literals (5);
	c = cnf.add_vars_with_literals (6);
	/*gint32 c0 = cnf.add_var ();
	LiteralVector c0vec;
	c0vec.push_back (c0);*/
	cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	//cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	cnf_gen_add (cnf, c, a, bv, 0, c0v);
	
	for (guint cv = 0; cv < 64; cv++)
	  if (check_cnf_satisfability (cnf, c, cv) !=
		(((av+bv+c0v)&0x3f) == cv))
	    return false;
      
        std::cout << "  " << av << '+' << bv << '+' << c0v << std::endl;
	cnf.clear ();
	a = cnf.add_vars_with_literals (5);
	c = cnf.add_vars_with_literals (5);
	/*gint32 c0 = cnf.add_var ();
	LiteralVector c0vec;
	c0vec.push_back (c0);*/
	cnf_gen_equal (cnf, 0, a, av, DEF_POSITIVE);
	//cnf_gen_equal (cnf, 0, c0vec, c0v, DEF_POSITIVE);
	cnf_gen_add (cnf, c, a, bv, 0, c0v);
	
	for (guint cv = 0; cv < 32; cv++)
	  if (check_cnf_satisfability (cnf, c, cv) !=
		(((av+bv+c0v)&0x1f) == cv))
	    return false;
      }
  
  /*cnf.clear ();
  a = cnf.add_vars_with_literals (5);
  c = cnf.add_vars_with_literals (6);
  gint32 c0 = cnf.add_var ();
  LiteralVector c0vec;
  c0vec.push_back (c0);
  cnf_gen_add (cnf, c, a, 0x12, 0, false);
  cnf.save_to_file ("adder-const-2.cnf");*/

#if 0
  {
    LiteralVector b, c2;
    guint64 bval = 0x23effedaf;
    cnf.clear ();
    //gint32 c0 = cnf.add_var ();
    a = cnf.add_vars_with_literals (32);
    b = cnf.add_vars_with_literals (32);
    c = cnf.add_vars_with_literals (33);
    c2 = cnf.add_vars_with_literals (33);
    cnf_gen_equal (cnf, 0, b, bval, DEF_POSITIVE);
    cnf_gen_add (cnf, c, a, b, 0, false);
    cnf_gen_add (cnf, c2, a, bval, 0, false);
    /* true if c!=c2 */
    cnf_gen_equal (cnf, 0, c, c2, DEF_NEGATIVE);
    cnf.save_to_file ("adder-test.cnf");
  }
#endif
  return true;
}
#endif

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (!gen_equal_test ())
  {
    std::cerr << "Gen Equal test is failed" << std::endl;
    return 1;
  }
  if (!gen_uint_less_test ())
  {
    std::cerr << "Gen UINT Less test is failed" << std::endl;
    return 1;
  }
  /*if (!gen_add_test ())
  {
    std::cerr << "Gen ADDER test is failed" << std::endl;
    return 1;
  }*/
  
  return 0;
}
