#! /bin/sh
make clean
echo "generating shell script to compile..."
make -n > temp.sh
dos2unix temp.sh
echo "contents of shell script: "
cat temp.sh
echo "compiling..."
sh -x ./temp.sh
echo "cleaning up..."
rm temp.sh
echo "$ ls tsp"
ls tsp --color
