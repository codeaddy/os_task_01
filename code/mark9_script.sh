#!/bin/bash

param1=$1
param2=$2

# запускаем первый файл
./mark9_part1 $param1 $param2 > log1.txt &

# запускаем второй файл
./mark9_part2 $param1 $param2 > log2.txt
