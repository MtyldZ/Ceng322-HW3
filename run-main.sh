#!/usr/bin/env bash
# UMUT YILDIZ 260201028
# Shell script to run compiled c program many times for each thread
# These results will be used to calculate more precise values
# USAGE: [ ./run-main.sh 1000 10 ]: [  <task amount> <repetition amount>]

out="out-$1.csv" # output file

rm $out
for i in 1 2 4 8 16; do # thread loop
  for _ in $(seq $2); do # execute $2 times with same values
    ./a.out $i $1 $out # execute compiled C code
  done
  sleep 1
done
