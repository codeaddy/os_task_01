#!/bin/bash

param1=$1
param2=$2

# запускаем первый файл
./mark8_part1 $param1 $param2 > log1.txt &

# запускаем второй файл
./mark8_part2 $param1 $param2 > log2.txt
