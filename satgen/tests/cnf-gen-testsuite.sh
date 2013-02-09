#!/bin/sh
####
# cnf-gen-testsuite.sh - CNF gen testsuite
# Author: Mateusz Szpakowski
# License: LGPL v2.0
####

function do_cnf_test
{
  local SUBST=$(echo $1 | sed -e "s/\\//\\\\\\//g")
  $(echo "${SATSOLVER}"| sed -e "s/\(.*\)\\%f\(.*\)/\1${SUBST}\2/")
  local retval=$?
  if [ ${retval} == 10 ]; then
    echo "FAILED"
    exit 1
  elif [ ${retval} == 20 ]; then
    echo "PASSED"
  else
    echo "UNKNOWN STATE"
    exit 2
  fi
  return 0
}

if [ $# == 0 ]; then
  echo "cnf-gen-testsuite.sh SATSOLVER [CNFGENDIR]"
  exit 0
fi

SATSOLVER="$1"
if [ $# == 2 ]; then
  CNFGENDIR="$2"
else
  CNFGENDIR="./"
fi

echo "Testing IF-THEN-ELSE"
for i in `seq 1 64`; do
  echo "ITE with $i bits"
  ${CNFGENDIR}/ite-gen-test $i $i $i 0 /tmp/test.cnf
  do_cnf_test /tmp/test.cnf
done

echo "Testing DADDA ADDER 2"
for i in `seq 1 10`; do
  INPUT=""
  for j in `seq 1 $i`; do
    INPUT+="$((${RANDOM}%8)) "
  done
  echo "Dadda with cols: ${INPUT}"
  echo "${INPUT}" > /tmp/input.txt
  ${CNFGENDIR}/dadda-gen-test-2 /tmp/input.txt /tmp/test.cnf
  do_cnf_test /tmp/test.cnf
  echo "Dadda with carry with cols: ${INPUT}"
  ${CNFGENDIR}/dadda-gen-test-2-c /tmp/input.txt /tmp/test.cnf
  do_cnf_test /tmp/test.cnf
done


echo "Testing MULTIPLY IMM"
for i in `seq 1 12`; do
  for j in `seq $i $((2*$i))`; do
    for k in `seq 1 2`; do
      BVALUE=${RANDOM}
      echo "Multiply with $i abits, $j outbits, with imm ${BVALUE}"
      ${CNFGENDIR}/multiply-gen-test $i ${BVALUE} $j /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done

echo "Testing SHIFTING"
for i in `seq 1 18`; do
  for j in `seq 1 5`; do
    for k in `seq $i $((2*$i))`; do
      echo "Shifting with $i abits, $j shiftbits, $k outbits"
      ${CNFGENDIR}/shift-gen-test $i $j $k 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      echo "Shifting with $i abits, $j shiftbits, $k outbits, lower=1"
      ${CNFGENDIR}/shift-gen-test $i $j $k 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done

echo "Testing SHIFTING IMM"
for j in `seq 1 7`; do
  for k in `seq 1 64`; do
    AVALUE=${RANDOM}
    echo "Shifting with imm ${AVALUE}, $j shiftbits, $k outbits"
    ${CNFGENDIR}/shift-imm-gen-test ${AVALUE} $j $k 0 /tmp/test.cnf
    do_cnf_test /tmp/test.cnf
    echo "Shifting with imm ${AVALUE}, $j shiftbits, $k outbits, lower=1"
    ${CNFGENDIR}/shift-imm-gen-test ${AVALUE} $j $k 1 /tmp/test.cnf
    do_cnf_test /tmp/test.cnf
  done
done

echo "Testing DIV"
for i in `seq 2 9`; do
  for j in `seq 2 $i`; do
    for k in `seq 2 $i`; do
      echo "Div uint with $i abits, $j bits, $k outbits"
      ${CNFGENDIR}/divmod-gen-test $i $j $k 0 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      echo "Div int with $i abits, $j bits, $k outbits"
      ${CNFGENDIR}/divmod-gen-test $i $j $k 1 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done

echo "Testing MODULO"
for i in `seq 2 9`; do
  for j in `seq 2 $i`; do
    for k in `seq 2 $j`; do
      echo "Mod uint with $i abits, $j bits, $k outbits"
      ${CNFGENDIR}/divmod-gen-test $i $j $k 0 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      echo "Mod int with $i abits, $j bits, $k outbits"
      ${CNFGENDIR}/divmod-gen-test $i $j $k 1 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done

echo "Testing DIV IMM"
for i in `seq 2 14`; do
  for j in `seq 2 $i`; do
    for k in `seq 1 3`; do
      BVALUE=$((${RANDOM}&((1<<$i)-1)))
      echo "Div uint with $i abits, imm ${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i ${BVALUE} $j 0 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      BVALUE=$((${RANDOM}&((1<<($i-1))-1)))
      echo "Div int with $i abits, imm ${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i ${BVALUE} $j 1 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      echo "Div int with $i abits, imm -${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i -${BVALUE} $j 1 1 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done

echo "Testing MODULO IMM"
for i in `seq 2 14`; do
  for j in `seq 2 $i`; do
    for k in `seq 1 3`; do
      BVALUE=$(((${RANDOM}&((1<<($i-1))-1))|(1<<($i-1))))
      echo "Mod uint with $i abits, imm ${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i ${BVALUE} $j 0 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      BVALUE=$(((${RANDOM}&((1<<($i-2))-1))|(1<<($i-2))))
      echo "Mod int with $i abits, imm ${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i ${BVALUE} $j 1 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
      BVALUE=$((${BVALUE}+1))
      echo "Mod int with $i abits, imm -${BVALUE}, $j outbits"
      ${CNFGENDIR}/divmod-imm-gen-test $i -${BVALUE} $j 1 0 /tmp/test.cnf
      do_cnf_test /tmp/test.cnf
    done
  done
done
