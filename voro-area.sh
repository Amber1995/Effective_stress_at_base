#!/bin/sh
g++ tess_area.cpp -o ./tess_area_moment
g++ table_voronoi.cpp -o ./table
for i in $(find . -iname "*.dat");do awk '{print $2}' $i> Radius.data;done
for i in $(find . -iname "*.dat"); do awk '{print $3" , " $4}' $i > $i.txt; ./table $i.txt; ./tess_area_moment $i;done
rm *.data
rm *.dat.txt





