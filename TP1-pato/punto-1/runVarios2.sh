#!/bin/bash


for i in 100 200 300 500; do 
 sed -i 's/^#define TIMES 100$/#define TIMES $i/' quadatric3.c
 ./compile.sh 2 
 sbatch run.sh quad 2 $i 
done
