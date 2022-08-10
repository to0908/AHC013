#!/bin/bash

cd `dirname $0`
g++ -std=gnu++1z -I . -O2 -Wall -Wfatal-errors -Wextra -W main.cpp
# g++ calc_score.cpp -o calc_score.out
# rm -rf score
# rm -rf out
# mkdir score
# mkdir out

st=0
en=0
procs=0
slep=1

calc() {
    ./a.out < in/$1.txt > out/$1.txt
    ./calc_score.out in/$1.txt out/$1.txt > score/$1.txt
}

export -f calc

# -J : 100ケース実行
while getopts "j:J" optKey; do
  case "$optKey" in
    J)
      en=99
      slep=4
      ;;
  esac
done

seq -f '%04g' $st $en | xargs -n1 -P $procs -I{} bash -c "calc {}" & sleep $slep

python3 score.py