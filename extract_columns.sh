#!/bin/sh
for i in $(find . -iname "*.dat"); do 
awk '{print $2}' $i> Radius.data;
filename=$(basename -- "$i")
awk '{print $2" " $3" " $4" " $24" "$25" " $26" " $27}' $i >"${filename%.*}.txt"; 
mv "${filename%.*}.txt" ./"${filename%.*}.dat"
done
#rm *.data
#rm *.dat.txt





