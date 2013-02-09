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
  echo -e "${TEXT}" > /tmp/test.sf
  ${SATGEN} -M spear-format -O /tmp/test.outmap /tmp/test.cnf /tmp/test.sf
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

function spear_equal_test
{
  local res a b resim aim bim sat expected op
  for op in "=" "/="; do
    for res in :0:i1 :1:i1 v:0:i1 v:1:i1; do
      for a in :0:i2 :1:i2 :2:i2 :3:i2 v:0:i2 v:1:i2 v:2:i2 v:3:i2; do
	for b in :0:i2 :1:i2 :2:i2 :3:i2 v:0:i2 v:1:i2 v:2:i2 v:3:i2; do
	  # get immediate values
	  resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  echo "Testing operation ${res} ${op} $a $b"
	  # determine good result
	  if [ ${aim} == ${bim} ]; then expected=1; else expected=0; fi
	  [ "${op}" == "/=" ] && expected=$((${expected}^1))
	  if [ ${expected} == ${resim} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	  gen_spearfile 0 ${res} ${op} $a $b
	  do_cnf_test /tmp/test.cnf ${sat}
	done
      done
    done
  done
}

function spear_logic_test
{
  local op res a b c expected resim aim bim cim sat
  for op in \& \| \^ "=>"; do
    for res in :0:i1 :1:i1 v:0:i1 v:1:i1; do
      for a in :0:i1 :1:i1 v:0:i1 v:1:i1; do
	for b in :0:i1 :1:i1 v:0:i1 v:1:i1; do
	  # get immediate values
	  resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  echo "Testing operation ${res} ${op} $a $b"
	  # deterime good result
	  if [ "${op}" != "=>" ]; then
	    expected=$((${aim}${op}${bim}))
	  else
	    # for implication (a^1|b) <=> a=>b
	    expected=$(((${aim}^1)|${bim}))
	  fi
	  if [ ${expected} == ${resim} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	  gen_spearfile 0 ${res} ${op} $a $b
	  do_cnf_test /tmp/test.cnf ${sat}
	done
      done
    done
  done
  
  for res in :0:i1 :1:i1 v:0:i1 v:1:i1; do
    for a in :0:i1 :1:i1 v:0:i1 v:1:i1; do
      # get immediate values
      resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
      aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
      echo "Testing operation ${res} ~ $a"
      expected=$((${aim}^1))
      if [ ${expected} == ${resim} ]; then
	sat="SAT"
      else
	sat="UNSAT"
      fi
      gen_spearfile 0 ${res} "~" $a
      do_cnf_test /tmp/test.cnf ${sat}
    done
  done
  
  for res in :0:i1 :1:i1 v:0:i1 v:1:i1; do
    for a in :0:i1 :1:i1 v:0:i1 v:1:i1; do
      for b in :0:i1 :1:i1 v:0:i1 v:1:i1; do
        for c in :0:i1 :1:i1 v:0:i1 v:1:i1; do
          # get immediate values
	  resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  cim=$(echo "${c}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  echo "Testing operation ${res} ite $a $b $c"
	  if [ ${aim} == 1 ]; then expected=$bim; else expected=$cim; fi
	  if [ ${expected} == ${resim} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	  gen_spearfile 0 ${res} ite $a $b $c
	  do_cnf_test /tmp/test.cnf ${sat}
	done
      done
    done
  done
}

function spear_cast_test
{
  local i op res a expected resim aim sat b c
  local VAL TVAL VAL2 TVAL2 BADTVAL BADTVAL2
  
  # TRUN operator
  for i in `seq 1 5`; do
    VAL=${RANDOM}
    VAL2=$((${VAL}+53))
    BADTVAL=$((${VAL}|128))
    BADTVAL2=$((${VAL2}|128))
    TVAL=$((${VAL}&127))
    TVAL2=$((${VAL2}&127))
    for res in :${TVAL}:i7 v:${TVAL}:i7 :${BADTVAL}:i7 v:${BADTVAL}:i7; do
      for a in :${VAL}:i15 v:${VAL}:i15 :${VAL2}:i15 v:${VAL2}:i15 \
        :${TVAL}:i15 v:${TVAL}:i15 :${TVAL2}:i15 v:${TVAL2}:i15; do
        # get immediate values
        resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	echo "Testing operation ${res} trun $a"
	if [ $((${resim}&127)) == $((${aim}&127)) ]; then
	  sat="SAT"
	else
	  sat="UNSAT"
	fi
	gen_spearfile 0 ${res} trun $a
	do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
  
  local resbits AVAL BVAL AVAL2 BVAL2
  # EXTR operator
  VAL=${RANDOM}
  VAL2=$((${VAL}+${RANDOM}))
  for a in :${VAL}:i10 v:${VAL}:i10 :${VAL2}:i10 v:${VAL2}:i10; do
    for b in `seq 1 9`; do
      for c in `seq $((b+1)) 10`; do
	  resbits=$(($c-$b))
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  # determine two  values for extraction two result
	  TVAL=$(((${VAL}>>$b) & ((1<<${resbits})-1)))
	  TVAL2=$(((${VAL2}>>$b) & ((1<<${resbits})-1)))
	  AVAL=$(((${aim}>>$b) & ((1<<${resbits})-1)))
	  
	  for res in :${TVAL}:i${resbits} v:${TVAL}:i${resbits} \
	      :${TVAL2}:i${resbits} v:${TVAL2}:i${resbits}; do
	    resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	    echo "Testing operation ${res} extr $a $b:i8 $c:i8"
	    if [ ${AVAL} == ${resim} ]; then
	      sat="SAT"
	    else
	      sat="UNSAT"
	    fi
	    gen_spearfile 0 ${res} extr $a :$b:i8 :$c:i8
	    do_cnf_test /tmp/test.cnf ${sat}
	  done
      done
    done
  done
  
  # CONCATENATE
  local dif rdif nores
  VAL=${RANDOM}
  VAL2=$(((${VAL}+${RANDOM})&0x7fff))
  for res in :${VAL}:i15 v:${VAL}:i15 :${VAL2}:i15 v:${VAL2}:i15; do
    for dif in `seq 1 2 14`; do
      rdif=$((15-${dif}))
      resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
      AVAL=$((${resim}>>${dif}))
      BVAL=$((${resim}&((1<<${dif})-1)))
      if [ ${resim} == ${VAL} ]; then nores=${VAL2}; else nores=${VAL}; fi
      
      AVAL2=$((${nores}>>${dif}))
      BVAL2=$((${nores}&((1<<${dif})-1)))
      echo ${nores} ${AVAL2} ${BVAL2}
      for a in :${AVAL}:i${rdif} v:${AVAL}:i${rdif} \
	  :${AVAL2}:i${rdif} v:${AVAL2}:i${rdif}; do
	for b in :${BVAL}:i${dif} v:${BVAL}:i${dif} \
	  :${BVAL2}:i${dif} v:${BVAL2}:i${dif}; do
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  echo "Testing operation ${res} conc $a $b"
	  if [ ${resim} == $(((${aim}<<${dif})|(${bim}&((1<<${dif})-1)))) ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	  gen_spearfile 0 ${res} conc $a $b
	  do_cnf_test /tmp/test.cnf ${sat}
	done
      done
    done
  done
  
  # ZEXT operator
  for i in `seq 1 5`; do
    VAL=$((${RANDOM}&4095))
    VAL2=$((${VAL}+2*4096))
    for res in :${VAL}:i16 v:${VAL}:i16 :${VAL2}:i16 v:${VAL2}:i16; do
      for a in :${VAL}:i12 v:${VAL}:i12 :${VAL2}:i12 v:${VAL2}:i12; do
	resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	echo "Testing operation ${res} zext $a"
	if [ ${resim} == $((${aim}&4095)) ]; then
	  sat="SAT"
	else
	  sat="UNSAT"
	fi
	gen_spearfile 0 ${res} zext $a
	do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
  # SEXT operator
  local SVAL SVAL2
  for i in `seq 1 5`; do
    # checking sign, and with bad and good sign extend
    VAL=$((${RANDOM}&4095))
    VAL2=$((${VAL}+3*8192))
    # 13-bit is sign
    SVAL=$((${VAL}|(15<<12)))
    SVAL2=$((${VAL2}|4096))
    for res in :${VAL}:i16 v:${VAL}:i16 :${VAL2}:i16 v:${VAL2}:i16 \
	:${SVAL}:i16 v:${SVAL}:i16 :${SVAL2}:i16 v:${SVAL2}:i16; do
      for a in :${VAL}:i13 v:${VAL}:i13 :${VAL2}:i13 v:${VAL2}:i13 \
	:${SVAL}:i13 v:${SVAL}:i13 :${SVAL2}:i13 v:${SVAL2}:i13; do
	resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	echo "Testing operation ${res} sext $a"
	if [ ${resim} == $(((${aim}&4095)|((${aim}&4096)*15))) ]; then
	  sat="SAT"
	else
	  sat="UNSAT"
	fi
	gen_spearfile 0 ${res} sext $a
	do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
}

function spear_compare_test
{
  local i op res a b expected resim aim bim sat
  local VAL VAL2 MVAL MVAL2
  VAL=${RANDOM}
  VAL=$((${VAL}&0xffff))
  MVAL=$(((${VAL}+32768)&0xffff))
  VAL2=$(((${VAL}+123)&0xffff))
  MVAL2=$(((${VAL2}+32768)&0xffff))
  
  for op in ult ule ugt uge slt sle sgt sge; do
    for res in :0:i1 v:0:i1 :1:i1 v:1:i1; do
      for a in :${VAL}:i16 v:${VAL}:i16 :${VAL2}:i16 v:${VAL2}:i16 \
          :${MVAL}:i16 v:${MVAL}:i16 :${MVAL2}:i16 v:${MVAL2}:i16; do
        for b in :${VAL}:i16 v:${VAL}:i16 :${VAL2}:i16 v:${VAL2}:i16 \
          :${MVAL}:i16 v:${MVAL}:i16 :${MVAL2}:i16 v:${MVAL2}:i16; do
	  
	  resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  echo "Testing operation ${res} ${op} $a $b"
	  if [ ${op:0:1} == 's' ]; then
	    # change signs for operands
	    # if signed comparison convert range 0x80-0x7f to 0x00-xff with
	    # change sign of values
	    aim=$((${aim}^32768))
	    bim=$((${bim}^32768))
	  fi
	  
	  expected=0
	  case ${op#[us]} in
	    lt)
	    [ ${aim} -lt ${bim} ] && expected=1
	    ;;
	    le)
	    [ ${aim} -le ${bim} ] && expected=1
	    ;;
	    gt)
	    [ ${aim} -gt ${bim} ] && expected=1
	    ;;
	    ge)
	    [ ${aim} -ge ${bim} ] && expected=1
	    ;;
	  esac
          if [ ${resim} == ${expected} ]; then
            sat="SAT"
          else
            sat="UNSAT"
          fi
          
	  gen_spearfile 0 ${res} "${op}" $a $b
	  do_cnf_test /tmp/test.cnf ${sat}
        done
      done
    done
  done
}

function spear_arith_test
{
  local i op res a b expected resim aim bim sat
  local resv av bv
  
  for op in "+" "-"; do
    for i in `seq 1 10`; do
      aim=$((${RANDOM}&0x7fff))
      bim=$((${RANDOM}&0x7fff))
      resim=$(((${aim}${op}${bim})&0x7fff))
      a=v:${aim}:i15
      b=v:${bim}:i15
      res=v:${resim}:i15
      echo "Testing operation ${res} ${op} $a $b"
      gen_spearfile 0 ${res} "${op}" $a $b
      do_cnf_test /tmp/test.cnf SAT
    done
    # immediate and vars combinations
    for resv in v i; do
      for av in v i; do
        for bv in v i; do
          if [ ${resv} == v ] &&[ ${av} == v ] && [ ${bv} == v ]; then
            continue;
          fi
          if [ ${resv} == i ] &&[ ${av} == i ] && [ ${bv} == i ]; then
            continue;
          fi
          for i in `seq 1 5`; do
            if [ ${resv} == i ]; then
              res=:$((${RANDOM}&0x7fff)):i15
            else
              res=v::i15
            fi
            if [ ${av} == i ]; then
              a=:$((${RANDOM}&0x7fff)):i15
            else
              a=v::i15
            fi
            if [ ${bv} == i ]; then
              b=:$((${RANDOM}&0x7fff)):i15
            else
              b=v::i15
            fi
            echo "Testing operation ${res} ${op} $a $b"
            gen_spearfile 1 ${res} "${op}" $a $b
            do_cnf_test /tmp/test.cnf UNSAT
          done
        done
      done
    done
  done
  
  # multiply
  local AVAL BVAL CVAL AVAL2 BVAL2 CVAL2
  for i in `seq 1 3`; do
    AVAL=$((${RANDOM}&0x7fff))
    BVAL=$((${RANDOM}&0x7fff))
    CVAL=$(((${AVAL}*${BVAL})&0x7fff))
    AVAL2=$(((${AVAL}+3)&0x7fff))
    BVAL2=$(((${BVAL}+5)&0x7fff))
    CVAL2=$(((${AVAL2}*${BVAL2})&0x7fff))
    
    for res in :${CVAL}:i15 v:${CVAL}:i15 :${CVAL2}:i15 v:${CVAL2}:i15; do
      for a in :${AVAL}:i15 v:${AVAL}:i15 :${AVAL2}:i15 v:${AVAL2}:i15; do
	for b in :${BVAL}:i15 v:${BVAL}:i15 :${BVAL2}:i15 v:${BVAL2}:i15; do
	  resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	  expected=$(((${aim}*${bim})&0x7fff))
	  if [ ${expected} == ${resim} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	  echo "Testing operation ${res} * $a $b"
	  gen_spearfile 0 ${res} "*" $a $b
	  do_cnf_test /tmp/test.cnf ${sat}
	done
      done
    done
  done
}

function spear_shift_test
{
  local i op res a b expected resim aim bim sat
  local VAL OUT BADOUT
  #VAL=$((${RANDOM}&0x7fff))
  for i in `seq 0 1`; do
    VAL=$((0x33ad|($i<<14)))
    for op in "<<" ">>l" ">>a"; do
      for bim in `seq 0 20`; do
	case "${op}" in
	  "<<")
	  OUT=$(((${VAL}<<${bim})&0x7fff))
	  ;;
	  ">>l")
	  OUT=$(((${VAL}>>${bim})&0x7fff))
	  ;;
	  ">>a")
	  if [ ${bim} -lt 15 ]; then
	    OUT=$(((${VAL}>>${bim})&0x7fff))
	    if [ $((${VAL}&0x4000)) == $((0x4000)) ]; then
	      OUT=$((${OUT}|(0x7fff&~((1<<(15-${bim}))-1))))
	    fi
	  else
	    if [ $((${VAL}&0x4000)) == $((0x4000)) ]; then
	      OUT=$((0x7fff))
	    else
	      OUT=0
	    fi
	  fi
	  ;;
	esac
	BADOUT=$((${OUT}^56))
	for res in :${OUT}:i15 v:${OUT}:i15 :${BADOUT}:i15 v:${BADOUT}:i15; do
	  for a in :${VAL}:i15 v:${VAL}:i15; do
	    for b in :${bim}:i8 v:${bim}:i8; do
	      resim=$(echo "${res}" | \
	           sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	      echo "Testing operation ${res} "${op}" $a $b"
	      if [ ${resim} == ${OUT} ]; then
		sat="SAT"
	      else
		sat="UNSAT"
	      fi
	      gen_spearfile 0 ${res} "${op}" $a $b
	      do_cnf_test /tmp/test.cnf ${sat}
	    done
	  done
	done
      done
    done
  done
}

function test_helper
{
  local res op a b sat
  while read res op a b sat; do
    echo ${res} "${op}" $a $b ${sat}
    echo ${res} "${op}" $a v$b ${sat}
    echo ${res} "${op}" v$a $b ${sat}
    echo ${res} "${op}" v$a v$b ${sat}
    echo v${res} "${op}" $a $b ${sat}
    echo v${res} "${op}" $a v$b ${sat}
    echo v${res} "${op}" v$a $b ${sat}
    echo v${res} "${op}" v$a v$b ${sat}
  done
}

function spear_divmod_test
{
  local i op res a b expected resim aim bim sat
  local AVAL BVAL OUT BADOUT
  # uint div/mod
  
  for i in `seq 1 10`; do
    AVAL=$((${RANDOM}&0x7fff))
    BVAL=$(((${RANDOM}+1)&0x7fff))
    for op in "/u" "%u"; do
      OUT=$((${AVAL}${op%u}${BVAL}))
      BADOUT=$((${OUT+3}&0x7fff))
      for resim in ${OUT} ${BADOUT}; do
        echo "Testing operation v:${resim}:i15 "${op}" v:${AVAL}:i15 v:${BVAL}:i15"
        if [ ${resim} == ${OUT} ]; then
          sat="SAT"
        else
          sat="UNSAT"
        fi
        gen_spearfile 0 v:${resim}:i15 "${op}" v:${AVAL}:i15 v:${BVAL}:i15
	do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
  
  local AVAL BVAL CVAL AVAL2 BVAL2 CVAL2
  for op in "/u" "%u"; do
    for i in `seq 1 3`; do
      AVAL=$((${RANDOM}&0x7fff))
      BVAL=$((${RANDOM}&0x7fff))
      CVAL=$(((${AVAL}${op%u}${BVAL})&0x7fff))
      AVAL2=$(((${AVAL}+3)&0x7fff))
      BVAL2=$(((${BVAL}+5)&0x7fff))
      CVAL2=$(((${AVAL2}${op%u}${BVAL2})&0x7fff))
      
      for res in :${CVAL}:i15 v:${CVAL}:i15 :${CVAL2}:i15 v:${CVAL2}:i15; do
	for a in :${AVAL}:i15 v:${AVAL}:i15 :${AVAL2}:i15 v:${AVAL2}:i15; do
	  for b in :${BVAL}:i15 v:${BVAL}:i15 :${BVAL2}:i15 v:${BVAL2}:i15; do
	    resim=$(echo "${res}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	    aim=$(echo "${a}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	    bim=$(echo "${b}" | sed -e "s/v\\?:\([[:digit:]]*\):i[[:digit:]]*/\1/")
	    expected=$(((${aim}${op%u}${bim})&0x7fff))
	    if [ ${expected} == ${resim} ]; then
	      sat="SAT"
	    else
	      sat="UNSAT"
	    fi
	    echo "Testing operation ${res} ${op} $a $b"
	    gen_spearfile 0 ${res} "${op}" $a $b
	    do_cnf_test /tmp/test.cnf ${sat}
	  done
	done
      done
    done
  done
  
  local resultval
  # signed operations
  { test_helper|{ while read res op a b sat; do
   echo "Testing operation ${res} ${op} $a $b"
   gen_spearfile 0 ${res} "${op}" $a $b
   do_cnf_test /tmp/test.cnf ${sat}
   done;};}<<EOF
:945:i17 /s :44416:i17 :47:i17 SAT
:945:i17 /u :44416:i17 :47:i17 SAT
:945:i17 /u :44449:i17 :47:i17 SAT
:130127:i17 /s :44417:i17 :131025:i17 SAT
:130127:i17 /s :86655:i17 :47:i17 SAT
:945:i17 /s :86651:i17 :131025:i17 SAT
:946:i17 /s :86657:i17 :131025:i17 UNSAT
:965:i17 /s :44415:i17 :47:i17 UNSAT
:945:i17 /s :86657:i17 :47:i17 UNSAT
:0:i17 /s :44415:i17 :0:i17 UNSAT
:0:i17 %s :44415:i17 :0:i17 UNSAT
:0:i17 /u :44415:i17 :0:i17 UNSAT
:0:i17 %u :44415:i17 :0:i17 UNSAT
:268:i17 %s :53458:i17 :591:i17 SAT
:268:i17 %s :53458:i17 :130481:i17 SAT
:130804:i17 %s :77614:i17 :591:i17 SAT
:130804:i17 %s :77614:i17 :130481:i17 SAT
:155:i16 %s :5435:i16 :176:i16 SAT
:155:i16 %s :5435:i16 :65360:i16 SAT
:65381:i16 %s :60101:i16 :176:i16 SAT
:65381:i16 %s :60101:i16 :65360:i16 SAT
:269:i17 %s :53458:i17 :591:i17 UNSAT
:268:i17 %s :53460:i17 :130481:i17 UNSAT
:130804:i17 %s :77613:i17 :591:i17 UNSAT
:130804:i17 %s :77613:i17 :130481:i17 UNSAT
:155:i16 %s :5435:i16 :177:i16 UNSAT
:155:i16 %s :5435:i16 :65361:i16 UNSAT
:65380:i16 %s :60101:i16 :176:i16 UNSAT
:65381:i16 %s :60102:i16 :65360:i16 UNSAT
:0:i1 /s :0:i1 :1:i1 SAT
:0:i3 %s :4:i3 :7:i3 SAT
EOF
  resultval=$?
  if [ ${resultval} != 0 ]; then exit 1; fi
  
  if false; then
  local aval bval resval
  for aim in `seq 8 15`; do
    for bim in `seq 0 15`; do
      for resim in `seq 0 15`; do
        if [ ${bim} != 0 ]; then
	  aval=${aim}
	  [ ${aim} -ge 8 ] && aval=$((${aval}-16))
	  bval=${bim}
	  [ ${bim} -ge 8 ] && bval=$((${bval}-16))
	  resval=${resim}
	  [ ${resim} -ge 8 ] && resval=$((${resval}-16))
	  expected=$((${aval}/${bval}))
	  if [ ${expected} == ${resval} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	else
	  expected="UNSAT"
	  sat="UNSAT"
	fi
        echo "Testing operation :${resim}:i4 /s :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "/s" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 /s :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "/s" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 /s v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "/s" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 /s v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "/s" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 /s :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "/s" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 /s :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "/s" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 /s v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "/s" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 /s v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "/s" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
  
  for aim in `seq 0 15`; do
    for bim in `seq 0 15`; do
      for resim in `seq 0 15`; do
        if [ ${bim} != 0 ]; then
	  aval=${aim}
	  [ ${aim} -ge 8 ] && aval=$((${aval}-16))
	  bval=${bim}
	  [ ${bim} -ge 8 ] && bval=$((${bval}-16))
	  resval=${resim}
	  [ ${resim} -ge 8 ] && resval=$((${resval}-16))
	  expected=$((${aval}%${bval}))
	  if [ ${expected} == ${resval} ]; then
	    sat="SAT"
	  else
	    sat="UNSAT"
	  fi
	else
	  expected="UNSAT"
	  sat="UNSAT"
	fi
        echo "Testing operation :${resim}:i4 %s :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%s" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 %s :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%s" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 %s v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%s" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation :${resim}:i4 %s v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 :${resim}:i4 "%s" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %s :${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%s" :${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %s :${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%s" :${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %s v:${aim}:i4 :${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%s" v:${aim}:i4 :${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
        echo "Testing operation v:${resim}:i4 %s v:${aim}:i4 v:${bim}:i4 ex=${expected}"
        gen_spearfile 0 v:${resim}:i4 "%s" v:${aim}:i4 v:${bim}:i4
        do_cnf_test /tmp/test.cnf ${sat}
      done
    done
  done
  fi
}

function spear_64bit_test
{
  local i op res a b expected resim aim bim sat
  local resultval
  { test_helper|{ while read res op a b sat; do
   echo "Testing operation ${res} ${op} $a $b"
   gen_spearfile 0 ${res} "${op}" $a $b
   do_cnf_test /tmp/test.cnf ${sat}
   done;};}<<EOF
:14348955666819872339:i64 + :18331345515114654733:i64 :14464354225414769222:i64 SAT
:14348955666819872339:i64 + :18331345515114654753:i64 :14464354225414769221:i64 UNSAT
:14579752784009666105:i64 - :14464354225414769222:i64 :18331345515114654733:i64 SAT
:14579752784009666105:i64 - :14464354225414769222:i64 :18331345515154654733:i64 UNSAT
:6207266676402771398:i64 * :8613759941065650286:i64 :17775524429395285173:i64 SAT
:6217266676402771398:i64 * :8613759941065650287:i64 :17775524429395285173:i64 UNSAT
:1:i1 ult :8548554545589589589:i64 :14636788376764757587:i64 SAT
:0:i1 ult :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:1:i1 ult :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:0:i1 ult :14636788376764757587:i64 :8548554545589589589:i64 SAT
:0:i1 ult :14636788376764757587:i64 :14636788376764757587:i64 SAT
:1:i1 ult :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:1:i1 ule :8548554545589589589:i64 :14636788376764757587:i64 SAT
:0:i1 ule :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:1:i1 ule :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:0:i1 ule :14636788376764757587:i64 :8548554545589589589:i64 SAT
:1:i1 ule :14636788376764757587:i64 :14636788376764757587:i64 SAT
:0:i1 ule :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:0:i1 ugt :8548554545589589589:i64 :14636788376764757587:i64 SAT
:1:i1 ugt :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:0:i1 ugt :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:1:i1 ugt :14636788376764757587:i64 :8548554545589589589:i64 SAT
:0:i1 ugt :14636788376764757587:i64 :14636788376764757587:i64 SAT
:1:i1 ugt :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:0:i1 uge :8548554545589589589:i64 :14636788376764757587:i64 SAT
:1:i1 uge :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:0:i1 uge :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:1:i1 uge :14636788376764757587:i64 :8548554545589589589:i64 SAT
:1:i1 uge :14636788376764757587:i64 :14636788376764757587:i64 SAT
:0:i1 uge :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:0:i1 slt :8548554545589589589:i64 :14636788376764757587:i64 SAT
:1:i1 slt :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:0:i1 slt :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:1:i1 slt :14636788376764757587:i64 :8548554545589589589:i64 SAT
:0:i1 slt :14636788376764757587:i64 :14636788376764757587:i64 SAT
:1:i1 slt :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:0:i1 sle :8548554545589589589:i64 :14636788376764757587:i64 SAT
:1:i1 sle :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:0:i1 sle :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:1:i1 sle :14636788376764757587:i64 :8548554545589589589:i64 SAT
:1:i1 sle :14636788376764757587:i64 :14636788376764757587:i64 SAT
:0:i1 sle :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:1:i1 sgt :8548554545589589589:i64 :14636788376764757587:i64 SAT
:0:i1 sgt :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:1:i1 sgt :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:0:i1 sgt :14636788376764757587:i64 :8548554545589589589:i64 SAT
:0:i1 sgt :14636788376764757587:i64 :14636788376764757587:i64 SAT
:1:i1 sgt :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:1:i1 sge :8548554545589589589:i64 :14636788376764757587:i64 SAT
:0:i1 sge :8548554545589589589:i64 :14636788376764757587:i64 UNSAT
:1:i1 sge :14636788376764757587:i64 :8548554545589589589:i64 UNSAT
:0:i1 sge :14636788376764757587:i64 :8548554545589589589:i64 SAT
:1:i1 sge :14636788376764757587:i64 :14636788376764757587:i64 SAT
:0:i1 sge :14636788376764757587:i64 :14636788376764757587:i64 UNSAT
:1706178163517685760:i64 << :12483898589589895897:i64 :44:i8 SAT
:0:i64 << :12483898589589895897:i64 :64:i8 SAT
:12483898589589895897:i64 << :12483898589589895897:i64 :0:i8 SAT
:1806178163517685760:i64 << :12483898589589895897:i64 :44:i8 UNSAT
:12:i64 << :12483898589589895897:i64 :64:i8 UNSAT
:12483898589589895898:i64 << :12483898589589895897:i64 :0:i8 UNSAT
:23253073151:i64 >>l :12483898589589895897:i64 :29:i8 SAT
:0:i64 >>l :12483898589589895897:i64 :64:i8 SAT
:12483898589589895897:i64 >>l :12483898589589895897:i64 :0:i8 SAT
:23253073152:i64 >>l :12483898589589895897:i64 :29:i8 UNSAT
:2:i64 >>l :12483898589589895897:i64 :64:i8 UNSAT
:12483898589589895897:i64 >>l :12483898589589895898:i64 :0:i8 UNSAT
:18446744062602886399:i64 >>a :12483898589589895897:i64 :29:i8 SAT
:18446744073709551615:i64 >>a :12483898589589895897:i64 :64:i8 SAT
:12483898589589895897:i64 >>a :12483898589589895897:i64 :0:i8 SAT
:18446744062622886401:i64 >>a :12483898589589895897:i64 :29:i8 UNSAT
:18446744053709551613:i64 >>a :12483898589589895897:i64 :64:i8 UNSAT
:12483898589589895697:i64 >>a :12483898589589895897:i64 :0:i8 UNSAT
:47617835:i64 >>a :6544545458985894977:i64 :37:i8 SAT
:0:i64 >>a :6544545458985894977:i64 :64:i8 SAT
:6544545458985894977:i64 >>a :6544545458985894977:i64 :0:i8 SAT
:369445708:i64 >>a :6544545458985894977:i64 :37:i8 UNSAT
:130:i64 >>a :6544545458985894977:i64 :64:i8 UNSAT
:6544545458988894997:i64 >>a :6544545458985894997:i64 :0:i8 UNSAT
:3:i64 /u :14949049095922949595:i64 :4938498893838985474:i64 SAT
:200672128:i64 /u :14949049095922949595:i64 :74494894867:i64 SAT
:200672128:i64 /u :14949049095922949597:i64 :74494894867:i64 SAT
:133552414405993173:i64 %u :14949049095922949595:i64 :4938498893838985474:i64 SAT
:17825782619:i64 %u :14949049095922949595:i64 :74494894867:i64 SAT
:18446744073708912323:i64 /s :17961850180225166729:i64 :758483895858:i64 SAT
:18446743625459915123:i64 %s :17961850180225166729:i64 :758483895858:i64 SAT
:639293:i64 /s :484893893484384887:i64 :758483895858:i64 SAT
:448249636493:i64 %s :484893893484384887:i64 :758483895858:i64 SAT
:18446744073708912323:i64 /s :484893893484384887:i64 :18446743315225655758:i64 SAT
:448249636493:i64 %s :484893893484384887:i64 :18446743315225655758:i64 SAT
:639293:i64 /s :17961850180225166729:i64 :18446743315225655758:i64 SAT
:18446743625459915123:i64 %s :17961850180225166729:i64 :18446743315225655758:i64 SAT
EOF
  resultval=$?
  if [ ${resultval} != 0 ]; then exit 1; fi
}


#gen_spearfile 0 :45:i8 "&" v::i8 :134:i8
spear_equal_test
spear_logic_test
spear_cast_test
spear_compare_test
spear_arith_test
spear_shift_test
spear_divmod_test
spear_64bit_test
