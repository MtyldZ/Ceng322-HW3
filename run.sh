#!/usr/bin/env bash
# UMUT YILDIZ 260201028
# executes run-main.sh with different task amounts
# USAGE: [ ./run.sh 10 ]: [ <repetition count> ]

bash run-main.sh 1000 $1
bash run-main.sh 10000 $1
bash run-main.sh 100000 $1

echo "Finished!: >> Check out.csv files"