#!/bin/sh
for i in $(find . -iname "*.txt");
do
filename=$(basename -- "$i")
sort $i>"${filename%.*}.dat";done
rm *.txt
mv *.dat ../Results





