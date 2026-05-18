#!/bin/bash


for i in 10 100 200 300; do 
 sbatch run.sh quadatric2-$i $i 
done
