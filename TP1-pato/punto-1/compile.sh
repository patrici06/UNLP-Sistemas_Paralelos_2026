#!/bin/bash

ARCHIVO=$1
OUT=$2

gcc -O3 -o ${OUT} ${ARCHIVO} -lm
