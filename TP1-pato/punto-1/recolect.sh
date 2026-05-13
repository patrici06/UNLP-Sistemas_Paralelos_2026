#!/bin/bash
for i in 10 100 200 300; do 
   echo "TIMES=$i" >> reporte-cluster.md
   cat output${i}.txt >> reporte-cluster.md
done
