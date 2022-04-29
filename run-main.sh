#!/usr/bin/env bash
# UMUT YILDIZ 260201028
# Shell script to run compiled c program many times for each thread
# These results will be used to calculate more precise values
# USAGE: [ ./run-main.sh 1000 ]

out="out-$1.csv" # output file

rm $out
for j in 1 2 4 8 16; do # thread loop
  for _ in {1..10}; do  # repetition count = 10
    ./a.out $j $i $out  # execute compiled C code
  done
  sleep 1
done
