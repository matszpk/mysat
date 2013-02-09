#!/bin/sh
####
# spear-testsuite.sh - spear format converter testsuite
# Author: Mateusz Szpakowski
# License: LGPL v2.0
####


# gen_spearfile CHECKWITHVARS RESOUT OP AIMM BIMM CIMM
# RESOUT,A,B,C in format "[v]:[value]:type - (with 'v' then use variable)"
function gen_spearfile
{
  local OP="$3"
  local ABITS AIMM AVAR BBITS BIMM BVAR CBITS CIMM CVAR
  
  # get variable, immediate value, and bitwidth
  local RESVAR=$(echo "$2" | sed -e "s/\(v\\?\):[[:digit:]]*:i[[:digit:]]*/\1/")
  local RESIMM=$(echo "$2" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
  local RESBITS=$(echo "$2" | sed -e "s/v\\?:[[:digit:]]*:i\([[:digit:]]*\)/\1/")
  if [ $# -ge 4 ]; then
    AVAR=$(echo "$4" | sed -e "s/\(v\\?\):[[:digit:]]*:i[[:digit:]]*/\1/")
    AIMM=$(echo "$4" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
    ABITS=$(echo "$4" | sed -e "s/v\\?:[[:digit:]]*:i\([[:digit:]]*\)/\1/")
  fi
  if [ $# -ge 5 ]; then
    BVAR=$(echo "$5" | sed -e "s/\(v\\?\):[[:digit:]]*:i[[:digit:]]*/\1/")
    BIMM=$(echo "$5" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
    BBITS=$(echo "$5" | sed -e "s/v\\?:[[:digit:]]*:i\([[:digit:]]*\)/\1/")
  fi
  if [ $# -ge 6 ]; then
    CVAR=$(echo "$6" | sed -e "s/\(v\\?\):[[:digit:]]*:i[[:digit:]]*/\1/")
    CIMM=$(echo "$6" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
    CBITS=$(echo "$6" | sed -e "s/v\\?:[[:digit:]]*:i\([[:digit:]]*\)/\1/")
  fi
  
  # determine next compare variable
  local COMPVAR
  local COMPBITS
  if [ $1 != 0 ]; then
    if [ x${RESIMM} == x ] && [ x${RESBITS} != x ]; then
      COMPVAR=res
      COMPBITS=${RESBITS}
    elif [ x${AIMM} == x ] && [ x${ABITS} != x ]; then
      COMPVAR=a
      COMPBITS=${ABITS}
    elif [ x${BIMM} == x ] && [ x${BBITS} != x ]; then
      COMPVAR=b
      COMPBITS=${BBITS}
    elif [ x${CIMM} == x ] && [ x${CBITS} != x ]; then
      COMPVAR=c
      COMPBITS=${CBITS}
    fi
  fi
  
  local TEXT="v 1.0\\ne 0\\nd"
  
  # put to text variables definition
  if [ x${RESVAR} == xv ] || [ $1 != 0 ]; then
    TEXT+=" res:i${RESBITS}"
  fi
  if [ x${AVAR} == xv ] || [ $1 != 0 ]; then
    TEXT+=" a:i${ABITS}"
  fi
  if [ x${BVAR} == xv ] || [ $1 != 0 ] && [ x${BBITS} != x ]; then
    TEXT+=" b:i${BBITS}"
  fi
  if [ x${CVAR} == xv ] || [ $1 != 0 ] && [ x${CBITS} != x ]; then
    TEXT+=" c:i${CBITS}"
  fi
  if [ $1 != 0 ] && [ x${COMPVAR} != x ]; then
    TEXT+=" ${COMPVAR}2:i${COMPBITS}"
  fi
  TEXT+="\\n"
  # assign immediate to variables if use variable in constraint with specified value
  if ([ x${RESVAR} == xv ] || [ $1 != 0 ]) && [ x${RESIMM} != x ]; then
    TEXT+="p = res ${RESIMM}:i${RESBITS}\\n"
  fi
  if ([ x${AVAR} == xv ] || [ $1 != 0 ]) && [ x${AIMM} != x ]; then
    TEXT+="p = a ${AIMM}:i${ABITS}\\n"
  fi
  if ([ x${BVAR} == xv ] || [ $1 != 0 ]) && [ x${BIMM} != x ]; then
    TEXT+="p = b ${BIMM}:i${BBITS}\\n"
  fi
  if ([ x${CVAR} == xv ] || [ $1 != 0 ]) && [ x${CIMM} != x ]; then
    TEXT+="p = c ${CIMM}:i${CBITS}\\n"
  fi
  # Constraint generating
  TEXT+="c"
  if [ x${RESVAR} == xv ]; then
    TEXT+=" res"
  else
    TEXT+=" ${RESIMM}:i${RESBITS}"
  fi
  TEXT+=" ${OP}"
  if [ x${AVAR} == xv ]; then
    TEXT+=" a"
  else
    TEXT+=" ${AIMM}:i${ABITS}"
  fi
  if [ x${BVAR} == xv ]; then
    TEXT+=" b"
  elif [ x${BBITS} != x ]; then
    TEXT+=" ${BIMM}:i${BBITS}"
  fi
  if [ x${CVAR} == xv ]; then
    TEXT+=" c"
  elif [ x${CBITS} != x ]; then
    TEXT+=" ${CIMM}:i${CBITS}"
  fi
  TEXT+="\\n"
  
  # constraint only with variables
  if [ $1 != 0 ] && [ x${COMPVAR} != x ]; then
    if [ ${COMPVAR} == res ]; then
      TEXT+="c res2 ${OP}"
    else
      TEXT+="c res ${OP}"
    fi
    [ x${ABITS} != x ] && TEXT+=" a"
    [ ${COMPVAR} == a ] && TEXT+='2'
    [ x${BBITS} != x ] && TEXT+=" b"
    [ ${COMPVAR} == b ] && TEXT+='2'
    [ x${CBITS} != x ] && TEXT+=" c"
    [ ${COMPVAR} == c ] && TEXT+='2'
    TEXT+="\\np /= ${COMPVAR} ${COMPVAR}2"
  fi
  
  # put to file and generate CNF
  echo -e "${TEXT}" > /tmp/test2.sf
  ${SATGEN} -M spear-format -O /tmp/test.outmap /tmp/test2.cnf /tmp/test2.sf
  SATGENRES=$?
  if [ ${SATGENRES} != 0 ]; then
    echo "satgen is FAILED"
    exit 1
  fi
}

function do_cnf_test
{
  local SUBST=$(echo $1 | sed -e "s/\\//\\\\\\//g")
  $(echo "${SATSOLVER}"| sed -e "s/\(.*\)\\%f\(.*\)/\1${SUBST}\2/")
  local retval=$?
  if [ ${retval} == 10 ]; then
    if [ $2 == "UNSAT" ]; then
      echo "FAILED"
      exit 1
    else
      echo "PASSED"
    fi
  elif [ ${retval} == 20 ]; then
    if [ $2 == "UNSAT" ]; then
      echo "PASSED"
    else
      echo "FAILED"
      exit 1
    fi
  else
    echo "UNKNOWN STATE"
    exit 2
  fi
  return 0
}

if [ $# == 0 ]; then
  echo "cnf-gen-testsuite.sh SATSOLVER [SATGEN]"
  exit 0
fi

SATSOLVER="$1"
if [ $# == 3 ]; then
  SATGEN="$2"
else
  SATGEN="./satgen"
fi

function spear_divmod_test
{
  local i op res a b expected resim aim bim sat
  local aval bval resval

  for aim in `seq 0 15`; do
    for bim in `seq 0 15`; do
      for resim in `seq 0 15`; do
        if [ ${bim} != 0 ]; then
	  expected=$((${aim}%${bim}))
	  if [ ${expected} == ${resim} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	else
	  expected="UNSAT"
	  sat="UNSAT"
	fi
        echo "Testing operation :${resim}:i4 %u :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%u" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation :${resim}:i4 %u :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%u" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation :${resim}:i4 %u v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%u" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation :${resim}:i4 %u v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%u" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %u :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%u" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %u :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%u" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %u v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%u" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %u v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%u" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test2.cnf ${sat}
      done
    done
  done
}

spear_divmod_test

