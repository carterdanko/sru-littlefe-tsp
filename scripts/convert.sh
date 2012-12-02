#!/bin/bash

# Usage: 
#  convert.sh <relative path of .tsp file to convert> <relative path for new 
#   input file>
#
# The purpose of this script is to convert a .tsp library file to the input
# file type that is used by our program.

tspData=$1
data=$2

sed '1,/NODE_COORD_SECTION/d' $tspData | cut -f3,4 -d" " > tempdataset
numCities=`wc -l tempdataset | tee | cut -f1 -d" "`
a=`expr $numCities`
sed -i "1i $a" tempdataset

head -`expr $numCities + 1` tempdataset > $data
rm tempdataset
