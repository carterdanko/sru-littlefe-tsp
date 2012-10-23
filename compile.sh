#! /bin/sh
make clean
echo "generating shell script to compile..."
make -n > temp.sh
dos2unix temp.sh
echo
echo "contents of shell script: "
cat temp.sh
echo
echo "compiling..."
sh -x ./temp.sh
echo
echo "cleaning up..."
rm temp.sh
echo
echo "$ ls tsp"
ls tsp --color
